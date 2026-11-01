#include "TerrainGenerator.h"

#include "AI/NavigationSystemBase.h"
#include "Components/BoxComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** Adds the scope's wall-clock time to a stage of FTerrainGenerateTimings. */
	struct FStageTimer
	{
		double& Ms;
		double Start = FPlatformTime::Seconds();
		explicit FStageTimer(double& InMs) : Ms(InMs) {}
		~FStageTimer() { Ms += (FPlatformTime::Seconds() - Start) * 1000.0; }
	};
}

namespace
{
	// WaterFloor box thickness, cm. Its top face sits at WaterLevel.
	constexpr float WaterFloorThickness = 100.f;

	// Fine end of the grid (MakeGrid): column spacing, and the longest stretch past a land-width corridor's end it covers, cm.
	constexpr float EndDetailCellSizeX = 250.f;
	constexpr float MaxEndDetailLength = 20000.f;

	// Shore mask (specs/*-water-surface-and-splash REQ-3): texels along the longer ground side, and the water depth, cm,
	// that maps to mask 0. The water material reads it through the ShoreMask / ShoreMaskBounds / ShoreMaskDepthRange parameters.
	constexpr int32 ShoreMaskMaxTexels = 512;
	constexpr float ShoreMaskDepthRange = 1000.f;

	/**
	 * G8 mask over the ground rectangle: 1 - depth / ShoreMaskDepthRange, where depth is the water above the ground
	 * (1 at the waterline and on land, 0 in deep water). The ground slopes into the sea, so depth stands in for the
	 * distance to the shore. The outer texel ring is 0 so the clamped sampler reads deep water past the ground.
	 */
	UTexture2D* BakeShoreMask(const TArray<float>& Heights, const FTerrainGrid& Grid, float SizeX, float SizeY, float WaterLevel)
	{
		const float Texel = FMath::Max(SizeX, SizeY) / ShoreMaskMaxTexels;
		const int32 Width = FMath::Clamp(FMath::CeilToInt(SizeX / Texel), 4, ShoreMaskMaxTexels);
		const int32 Height = FMath::Clamp(FMath::CeilToInt(SizeY / Texel), 4, ShoreMaskMaxTexels);

		TArray<uint8> Pixels;
		Pixels.SetNumZeroed(Width * Height);
		for (int32 Y = 1; Y < Height - 1; ++Y)
		{
			const float LocalY = -SizeY * 0.5f + (Y + 0.5f) * SizeY / Height;
			for (int32 X = 1; X < Width - 1; ++X)
			{
				const float LocalX = -SizeX * 0.5f + (X + 0.5f) * SizeX / Width;
				const float Depth = WaterLevel - FTerrainHeightField::SampleGrid(Heights, Grid, SizeX, SizeY, LocalX, LocalY);
				Pixels[Y * Width + X] = static_cast<uint8>(FMath::RoundToInt(255.f * (1.f - FMath::Clamp(Depth / ShoreMaskDepthRange, 0.f, 1.f))));
			}
		}

		UTexture2D* Mask = UTexture2D::CreateTransient(Width, Height, PF_G8, NAME_None, Pixels);
		if (Mask)
		{
			Mask->SRGB = false;
			Mask->Filter = TF_Bilinear;
			Mask->AddressX = TA_Clamp;
			Mask->AddressY = TA_Clamp;
			Mask->UpdateResource();
		}
		return Mask;
	}
}

ATerrainGenerator::ATerrainGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	Ground = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Ground"));
	if (!Ground)
	{
		return;
	}

	Ground->bUseAsyncCooking = true;
	Ground->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	RootComponent = Ground;

	Water = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Water"));
	if (!Water)
	{
		return;
	}

	Water->SetupAttachment(Ground);
	Water->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Water->SetCanEverAffectNavigation(false);
	Water->CastShadow = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		Water->SetStaticMesh(PlaneMesh.Object);
	}

	// Same profile as Ground, so water and land can't drift apart: anything that treats WorldStatic as ground
	// (ram damage, ground traces, projectile impacts) treats the water surface the same way.
	WaterFloor = CreateDefaultSubobject<UBoxComponent>(TEXT("WaterFloor"));
	if (!WaterFloor)
	{
		return;
	}

	WaterFloor->SetupAttachment(Ground);
	WaterFloor->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	WaterFloor->SetGenerateOverlapEvents(false);
	WaterFloor->SetCanEverAffectNavigation(false);
	WaterFloor->SetHiddenInGame(true);
}

void ATerrainGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (!bGenerateOnConstruction)
	{
		return;
	}

	// Ground and water only: decoration components are created on demand (Generate / GenerateDecorations buttons),
	// not from construction, which reruns on every drag.
	BuildGroundMesh();
	UpdateWater();
}

void ATerrainGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (bRandomSeedOnBeginPlay)
	{
		Seed = FMath::Rand();
		Generate();
	}
	else if (TerrainType != ETerrainType::WaterOnly && (Heights.Num() != NumX * NumY || NumX < 2))
	{
		Generate();
	}
	else
	{
		// Saved terrains: the water's shore mask is transient, so build it from the saved heights.
		UpdateWater();
	}
	SpawnStream.Initialize(Seed);
}

#if WITH_EDITOR
void ATerrainGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATerrainGenerator, TerrainType))
	{
		ApplyTypeDefaults();
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void ATerrainGenerator::ApplyTypeDefaults()
{
	switch (TerrainType)
	{
	case ETerrainType::Island:
		MinLandHeight = -20.f;
		bWater = true;
		break;
	case ETerrainType::Continent:
		MinLandHeight = -1000.f;
		bWater = false;
		break;
	case ETerrainType::WaterOnly:
		bWater = true;
		break;
	}
}

FTerrainHeightParams ATerrainGenerator::MakeHeightParams() const
{
	FTerrainHeightParams Params;
	Params.Type = TerrainType;
	Params.SizeX = SizeX;
	Params.SizeY = SizeY;
	Params.MaxHeight = MaxHeight;
	Params.MinLandHeight = MinLandHeight;
	Params.Seed = Seed;
	Params.NoiseScale = NoiseScale;
	Params.Octaves = Octaves;
	Params.Persistence = Persistence;
	Params.Lacunarity = Lacunarity;
	Params.Flatness = Flatness;
	Params.RidgeWeight = RidgeWeight;
	Params.RidgeStart = RidgeStart;
	Params.ShoreWidth = ShoreWidth;
	Params.SeaDepth = SeaDepth;
	Params.CoastlineNoise = CoastlineNoise;
	Params.WaterLevel = WaterLevel;
	Params.Corridors = Corridors;
	return Params;
}

FTerrainGrid ATerrainGenerator::MakeGrid() const
{
	float EndDetailLength = 0.f;
	if (TerrainType == ETerrainType::Island)
	{
		for (const FTerrainCorridor& Corridor : Corridors)
		{
			const float Length = SizeX * 0.5f - FMath::Max(Corridor.StartX, Corridor.EndX);
			if (Corridor.bDefineLandWidth && Length > 0.f && Length <= MaxEndDetailLength)
			{
				EndDetailLength = FMath::Max(EndDetailLength, Length);
			}
		}
	}
	FTerrainGrid Grid = FTerrainGrid::Make(SizeX, SizeY, CellSizeX > 0.f ? CellSizeX : CellSize, CellSizeY > 0.f ? CellSizeY : CellSize,
		EndDetailLength, EndDetailCellSizeX);
	// A cliff face is exactly one cell, with columns on both edges; a rough cliff's zone has its own columns only.
	Grid.InsertColumns(FTerrainHeightField::CliffColumnXs(Corridors), SizeX, 100.f, FTerrainHeightField::CliffZoneSpans(Corridors));
	// Fewer rows on background ground (lane-ground-dressing T29).
	if (CoarseCellSizeY > Grid.StepY && Corridors.IsValidIndex(DecorationCorridorIndex))
	{
		const FTerrainCorridor& Lane = Corridors[DecorationCorridorIndex];
		// The land-width edges stay rows: the lane check samples them exactly (dry minimum).
		TArray<float> KeepYs;
		if (Lane.bDefineLandWidth)
		{
			const float MinusEdge = Lane.CenterY - Lane.LandMinusY;
			const float PlusEdge = Lane.CenterY + Lane.LandPlusY;
			KeepYs.Add(MinusEdge);
			KeepYs.Add(PlusEdge);
			// Finding #77: the shore (beach, shelf, drop to the seabed) keeps every row from a bit inside the land edge out to
			// the mesh border, or the drop is one long 15 m triangle whose straight edge shows through the water.
			constexpr float ShoreFineInland = 1000.f;
			const float HalfY = SizeY * 0.5f;
			for (float Y = -HalfY; Y <= HalfY + UE_KINDA_SMALL_NUMBER; Y += Grid.StepY)
			{
				if (Y <= MinusEdge + ShoreFineInland || Y >= PlusEdge - ShoreFineInland)
				{
					KeepYs.Add(Y);
				}
			}
		}
		Grid.ThinRows(Lane.CenterY - FineRowsMinusY, Lane.CenterY + FineRowsPlusY, FMath::RoundToInt(CoarseCellSizeY / Grid.StepY), SizeY,
			KeepYs);
	}
	return Grid;
}

uint32 ATerrainGenerator::MakeGridKey() const
{
	uint32 Key = HashCombine(GetTypeHash(SizeX), GetTypeHash(SizeY));
	for (const float Value : { CellSize, CellSizeX, CellSizeY, StepX, StepY, CoarseCellSizeY, FineRowsMinusY, FineRowsPlusY })
	{
		Key = HashCombine(Key, GetTypeHash(Value));
	}
	Key = HashCombine(Key, HashCombine(GetTypeHash(NumX), GetTypeHash(NumY)));
	Key = HashCombine(Key, HashCombine(GetTypeHash(static_cast<uint8>(TerrainType)), GetTypeHash(DecorationCorridorIndex)));
	for (const FTerrainCorridor& Corridor : Corridors)
	{
		Key = HashCombine(Key, HashCombine(GetTypeHash(Corridor.StartX), GetTypeHash(Corridor.EndX)));
		Key = HashCombine(Key, GetTypeHash(Corridor.CenterY));
		Key = HashCombine(Key, GetTypeHash(Corridor.bDefineLandWidth));
		Key = HashCombine(Key, HashCombine(GetTypeHash(Corridor.CliffRoughness > 0.f), HashCombine(GetTypeHash(Corridor.CliffLipDepth),
			GetTypeHash(Corridor.CliffToeDepth))));
		for (const FTerrainCliff& Cliff : Corridor.Cliffs)
		{
			Key = HashCombine(Key, HashCombine(GetTypeHash(Cliff.StartX), HashCombine(GetTypeHash(Cliff.Run), GetTypeHash(Cliff.Rise))));
		}
	}
	return Key;
}

const FTerrainGrid& ATerrainGenerator::GetGrid() const
{
	const uint32 Key = MakeGridKey();
	if (bHasCachedGrid && CachedGridKey == Key)
	{
		return CachedGrid;
	}
	const FTerrainGrid Grid = MakeGrid();
	CachedGrid = Grid.NumX == NumX && Grid.NumY == NumY ? Grid : FTerrainGrid{ NumX, NumY, StepX, StepY };
	CachedGridKey = Key;
	bHasCachedGrid = true;
	return CachedGrid;
}

void ATerrainGenerator::Generate()
{
	GenerateGround();
	GenerateDecorations();
}

void ATerrainGenerator::GenerateGround()
{
	GenerateTimings = FTerrainGenerateTimings();
	// GenerateDecorations() ends with the ground darkness, contact patches included (lane-ground-dressing T23).
	BuildGroundMesh(/*bUpdateDarkness*/ false);
	UpdateWater();
}

bool ATerrainGenerator::HasGroundCollision() const
{
	if (Ground == nullptr || Ground->GetNumSections() == 0)
	{
		return true;
	}
	// Until an async cook finishes, the component has no collision shapes and the trace misses.
	const FVector Centre = GetActorLocation();
	FHitResult Hit;
	return Ground->LineTraceComponent(Hit, Centre + FVector(0.f, 0.f, 1.0e6f), Centre - FVector(0.f, 0.f, 1.0e6f),
		FCollisionQueryParams(SCENE_QUERY_STAT(TerrainGroundCollisionReady)));
}

void ATerrainGenerator::BuildGroundMesh(bool bUpdateDarkness)
{
	SpawnStream.Initialize(Seed);
	Ground->bUseAsyncCooking = bAsyncCollisionCooking;
	Ground->ClearAllMeshSections();

	if (TerrainType == ETerrainType::WaterOnly)
	{
		Heights.Reset();
		NumX = NumY = 0;
		StepX = StepY = 0.f;
		return;
	}

	const FTerrainGrid Grid = MakeGrid();
	NumX = Grid.NumX;
	NumY = Grid.NumY;
	StepX = Grid.StepX;
	StepY = Grid.StepY;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TerrainGenerator_HeightField);
		FStageTimer Timer(GenerateTimings.HeightFieldMs);
		FTerrainHeightField::Build(MakeHeightParams(), Grid, Heights);
	}
	const double ArraysStart = FPlatformTime::Seconds();

	const int32 NumVerts = NumX * NumY;
	TArray<FVector> Vertices;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FLinearColor> Colors;
	TArray<FProcMeshTangent> Tangents;
	TArray<int32> Triangles;
	Vertices.Reserve(NumVerts);
	Normals.Reserve(NumVerts);
	UV0.Reserve(NumVerts);
	Colors.Reserve(NumVerts);
	Tangents.Reserve(NumVerts);
	Triangles.Reserve((NumX - 1) * (NumY - 1) * 6);

	auto H = [this](int32 X, int32 Y) { return Heights[Y * NumX + X]; };
	const FTerrainCorridor* PathCorridor = Corridors.IsValidIndex(PathCorridorIndex) ? &Corridors[PathCorridorIndex] : nullptr;

	// Columns on a cliff face's edge and the side of their flat neighbour (-1 / +1, 0 = not a cliff column). Their
	// steepness (vertex colour G) comes from that flat side, so the rock layer doesn't bleed a cell into the flat ground;
	// the face itself gets its rock from the material (geometric normal). A rough cliff's zone (CliffZone) counts as the
	// face: its outer columns take the flat side, and the columns inside it the face's mean steepness (CliffZoneSteep,
	// < 0 = none), so ledges and the broken lip stay rock too.
	TArray<int8> CliffSide;
	CliffSide.Init(0, NumX);
	// Finding #74: across the lip and the toe the zone's rock eases out (CliffZoneRamp, 1 on the face, toward 0 at the outer
	// columns), with a per-row jitter so the grass/rock boundary isn't a straight line.
	TArray<float> CliffZoneSteep;
	CliffZoneSteep.Init(-1.f, NumX);
	TArray<float> CliffZoneRamp;
	CliffZoneRamp.Init(1.f, NumX);
	TArray<float> RampXs;
	TArray<float> Ramps;
	constexpr float CliffRampJitter = 0.25f;
	for (const FTerrainCorridor& Corridor : Corridors)
	{
		for (const FTerrainCliff& Cliff : Corridor.Cliffs)
		{
			const FVector2D Zone = FTerrainHeightField::CliffZone(Corridor, Cliff);
			const int32 Top = Grid.FindColumn(Zone.X, SizeX);
			const int32 Bottom = Grid.FindColumn(Zone.Y, SizeX);
			if (Top != INDEX_NONE && Top > 0)
			{
				CliffSide[Top] = -1;
			}
			if (Bottom != INDEX_NONE && Bottom < NumX - 1)
			{
				CliffSide[Bottom] = 1;
			}
			if (Top != INDEX_NONE && Bottom != INDEX_NONE)
			{
				const float Steep = 1.f - FMath::Cos(FMath::Atan(FMath::Abs(Cliff.Rise) / FMath::Max(Cliff.Run, 1.f)));
				FTerrainHeightField::CliffZoneRockRamp(Corridor, Cliff, RampXs, Ramps);
				for (int32 K = 0; K < RampXs.Num(); ++K)
				{
					const int32 X = Grid.FindColumn(RampXs[K], SizeX);
					if (X > Top && X < Bottom)
					{
						CliffZoneSteep[X] = Steep;
						CliffZoneRamp[X] = Ramps[K];
					}
				}
			}
		}
	}

	for (int32 Y = 0; Y < NumY; ++Y)
	{
		for (int32 X = 0; X < NumX; ++X)
		{
			const float PX = Grid.ColumnX(X, SizeX);
			const float PY = Grid.RowY(Y, SizeY);
			const float PZ = H(X, Y);

			// Central differences for smooth shading.
			const int32 X0 = FMath::Max(X - 1, 0), X1 = FMath::Min(X + 1, NumX - 1);
			const int32 Y0 = FMath::Max(Y - 1, 0), Y1 = FMath::Min(Y + 1, NumY - 1);
			const float DHdX = (H(X1, Y) - H(X0, Y)) / (Grid.ColumnX(X1, SizeX) - Grid.ColumnX(X0, SizeX));
			const float DHdY = (H(X, Y1) - H(X, Y0)) / (Grid.RowY(Y1, SizeY) - Grid.RowY(Y0, SizeY));
			const FVector Normal = FVector(-DHdX, -DHdY, 1.f).GetSafeNormal();

			Vertices.Add(FVector(PX, PY, PZ));
			Normals.Add(Normal);
			UV0.Add(FVector2D(PX, PY) / UVTileSize);
			// R = height above water [0..1], G = steepness [0..1]: blend sand/grass/rock in the material.
			// B = darkness [0..1], filled in by UpdateGroundDarkness below. A = 1 - path mask (1 = no path).
			const float HeightAlpha = MaxHeight > 0.f ? FMath::Clamp((PZ - WaterLevel) / MaxHeight, 0.f, 1.f) : 0.f;
			const float Path = PathCorridor ? FTerrainHeightField::PathWeight(*PathCorridor, PathHalfWidth, PX, PY) : 0.f;
			float SteepZ = Normal.Z;
			if (CliffSide[X] != 0)
			{
				const int32 Flat = X + CliffSide[X];
				const float FlatDHdX = (H(Flat, Y) - H(X, Y)) / (Grid.ColumnX(Flat, SizeX) - PX);
				SteepZ = FVector(-FlatDHdX, -DHdY, 1.f).GetSafeNormal().Z;
			}
			float Steepness = 1.f - SteepZ;
			if (CliffZoneSteep[X] >= 0.f)
			{
				float Ramp = CliffZoneRamp[X];
				if (Ramp < 1.f)
				{
					Ramp = FMath::Clamp(Ramp + CliffRampJitter * FMath::PerlinNoise1D(PY / 1000.f + X * 0.618f), 0.f, 1.f);
				}
				Steepness = FMath::Max(Steepness, CliffZoneSteep[X] * Ramp);
			}
			Colors.Add(FLinearColor(HeightAlpha, Steepness, 0.f, 1.f - Path));
			Tangents.Add(FProcMeshTangent(FVector(1.f, 0.f, DHdX).GetSafeNormal(), false));
		}
	}

	// Two triangles per cell, wound so faces point up (+Z). Diagonal runs V1 -> V2 (FTerrainHeightField::SampleGrid matches).
	for (int32 Y = 0; Y < NumY - 1; ++Y)
	{
		for (int32 X = 0; X < NumX - 1; ++X)
		{
			const int32 V0 = Y * NumX + X;
			const int32 V1 = V0 + 1;
			const int32 V2 = V0 + NumX;
			const int32 V3 = V2 + 1;
			Triangles.Append({ V0, V2, V1 });
			Triangles.Append({ V1, V2, V3 });
		}
	}

	GenerateTimings.MeshArraysMs += (FPlatformTime::Seconds() - ArraysStart) * 1000.0;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TerrainGenerator_MeshSection);
		FStageTimer Timer(GenerateTimings.MeshSectionMs);
		Ground->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, Colors, Tangents, /*bCreateCollision*/ true);
	}
	// Keeps the contact patches of decorations that survive a ground rebuild (construction reruns).
	if (bUpdateDarkness)
	{
		UpdateGroundDarkness();
	}
	if (GroundMaterial)
	{
		Ground->SetMaterial(0, GroundMaterial);
	}

	// Lets a dynamic navmesh rebuild over the new ground.
	FNavigationSystem::UpdateComponentData(*Ground);
}

void ATerrainGenerator::UpdateWater()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TerrainGenerator_Water);
	FStageTimer Timer(GenerateTimings.WaterMs);
	// Engine plane mesh is 100x100 cm.
	const float WaterSize = FMath::Max(SizeX, SizeY) * WaterExtent;
	Water->SetRelativeLocation(FVector(0.f, 0.f, WaterLevel));
	Water->SetRelativeScale3D(FVector(WaterSize / 100.f, WaterSize / 100.f, 1.f));
	Water->SetVisibility(bWater);

	// Shallows and shore foam: a transient instance carries the mask baked from the current heights (not saved; rebuilt
	// on construction, Generate and BeginPlay). Without ground the plain material reads as open sea.
	UMaterialInterface* Material = WaterMaterial;
	if (WaterMaterial && bWater && TerrainType != ETerrainType::WaterOnly && NumX >= 2 && NumY >= 2 && Heights.Num() == NumX * NumY)
	{
		if (UTexture2D* Mask = BakeShoreMask(Heights, GetGrid(), SizeX, SizeY, WaterLevel))
		{
			UMaterialInstanceDynamic* Instance = UMaterialInstanceDynamic::Create(WaterMaterial, this);
			Instance->SetFlags(RF_Transient);
			const FVector Origin = GetActorLocation();
			Instance->SetTextureParameterValue(TEXT("ShoreMask"), Mask);
			Instance->SetVectorParameterValue(TEXT("ShoreMaskBounds"),
				FLinearColor(Origin.X - SizeX * 0.5f, Origin.Y - SizeY * 0.5f, 1.f / SizeX, 1.f / SizeY));
			Instance->SetScalarParameterValue(TEXT("ShoreMaskDepthRange"), ShoreMaskDepthRange);
			Material = Instance;
		}
	}
	if (Material)
	{
		Water->SetMaterial(0, Material);
	}

	WaterFloor->SetRelativeLocation(FVector(0.f, 0.f, WaterLevel - WaterFloorThickness * 0.5f));
	WaterFloor->SetBoxExtent(FVector(WaterSize * 0.5f, WaterSize * 0.5f, WaterFloorThickness * 0.5f));
	WaterFloor->SetCollisionEnabled(bWater ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}

void ATerrainGenerator::ClearDecorations()
{
	for (TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>>* Components : { &DecorationComponents })
	{
		for (UHierarchicalInstancedStaticMeshComponent* Component : *Components)
		{
			if (IsValid(Component))
			{
				RemoveInstanceComponent(Component);
				Component->DestroyComponent();
			}
		}
		Components->Reset();
	}
}

void ATerrainGenerator::UpdateGroundDarkness()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TerrainGenerator_GroundDarkness);
	FStageTimer Timer(GenerateTimings.GroundDarknessMs);
	FProcMeshSection* Section = Ground->GetProcMeshSection(0);
	if (Section == nullptr || NumX < 2 || NumY < 2 || Heights.Num() != NumX * NumY || Section->ProcVertexBuffer.Num() != Heights.Num())
	{
		return;
	}

	const FTerrainGrid Grid = GetGrid();
	TArray<float> Darkness;
	FTerrainHeightField::ComputeOcclusion(Heights, Grid, SizeX, SizeY, OcclusionRadius, OcclusionDepth, Darkness);
	if (ContactDarkening > 0.f)
	{
		for (const UHierarchicalInstancedStaticMeshComponent* Component : DecorationComponents)
		{
			const UStaticMesh* Mesh = IsValid(Component) ? Component->GetStaticMesh() : nullptr;
			if (Mesh == nullptr)
			{
				continue;
			}
			// Centred on the bounds centre, where the scatter put the sample. Unusable bounds get a 1 m reach.
			const FBox Box = Mesh->GetBounds().GetBox();
			const bool bUsable = Box.IsValid && !Box.ContainsNaN() && Box.GetExtent().GetMax() < 1.0e6 && !Box.GetExtent().IsNearlyZero();
			const FVector CenterXY = bUsable ? FVector(Box.GetCenter().X, Box.GetCenter().Y, 0.f) : FVector::ZeroVector;
			const float Reach = bUsable ? FVector2D(Box.GetExtent()).Size() : 100.f;
			for (int32 Instance = 0; Instance < Component->GetInstanceCount(); ++Instance)
			{
				FTransform Transform;
				Component->GetInstanceTransform(Instance, Transform, /*bWorldSpace*/ false);
				FTerrainHeightField::AddContactDarkening(Grid, SizeX, SizeY, FVector2D(Transform.TransformPosition(CenterXY)),
					Reach * Transform.GetScale3D().X * ContactRadiusScale, ContactDarkening, Darkness);
			}
		}
	}

	// Only B changes: positions, normals and collision stay as built.
	for (int32 Index = 0; Index < Darkness.Num(); ++Index)
	{
		Section->ProcVertexBuffer[Index].Color.B = (uint8)FMath::RoundToInt(FMath::Clamp(Darkness[Index], 0.f, 1.f) * 255.f);
	}
	Ground->MarkRenderStateDirty();
}

void ATerrainGenerator::GenerateDecorations()
{
	ClearDecorations();
	if (TerrainType == ETerrainType::WaterOnly || NumX < 2 || NumY < 2 || Heights.Num() != NumX * NumY)
	{
		return;
	}

	if (DecorationSet == nullptr)
	{
		// The old decorations are gone, and so are their contact patches.
		UpdateGroundDarkness();
		return;
	}

	const TArray<FTerrainDecorationEntry>& Entries = DecorationSet->Entries;
	TArray<FTerrainDecorationMeshInfo> MeshInfo;
	MeshInfo.SetNum(Entries.Num());
	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		if (const UStaticMesh* Mesh = Entries[Index].Mesh)
		{
			const FBox Box = Mesh->GetBounds().GetBox();
			// Some imports report empty or garbage bounds (non-finite, kilometres wide): treat them as a point and say so.
			constexpr double MaxSaneExtent = 1.0e6;
			if (!Box.IsValid || Box.ContainsNaN() || Box.GetExtent().GetMax() > MaxSaneExtent || Box.GetExtent().IsNearlyZero())
			{
				UE_LOG(LogTemp, Warning, TEXT("TerrainGenerator %s: decoration mesh %s has unusable bounds (%s); placed as a point."),
					*GetName(), *Mesh->GetName(), *Box.ToString());
				continue;
			}
			// Measured from the bounds centre, which the scatter puts on the sample (see FTerrainDecorationMeshInfo).
			MeshInfo[Index].CenterXY = FVector2D(Box.GetCenter());
			MeshInfo[Index].RadiusXY = FVector2D(Box.GetExtent()).Size();
			MeshInfo[Index].Height = FMath::Max(0.f, Box.Max.Z);
		}
	}

	FTerrainCorridor Lane;
	if (Corridors.IsValidIndex(DecorationCorridorIndex))
	{
		Lane = Corridors[DecorationCorridorIndex];
	}
	else
	{
		Lane.CenterY = 0.f;
		Lane.StartX = -SizeX * 0.5f;
		Lane.EndX = SizeX * 0.5f;
	}

	FTerrainDecorationGround GroundQuery;
	GroundQuery.HeightAboveWater = [this](const FVector2D& P) { return LocalHeightAboveWater(P); };
	GroundQuery.Normal = [this](const FVector2D& P) { return LocalNormal(P); };
	GroundQuery.WaterZ = WaterLevel;
	GroundQuery.bHasWater = bWater;

	FRandomStream Stream(HashCombine(GetTypeHash(Seed), GetTypeHash(DecorationSeedSalt)));
	TArray<TArray<FTransform>> PerEntry;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TerrainGenerator_Scatter);
		FStageTimer Timer(GenerateTimings.ScatterMs);
		FTerrainDecorationScatter::Scatter(*DecorationSet, MeshInfo, Lane, GroundQuery, Stream, PerEntry);
	}
	const double InstancesStart = FPlatformTime::Seconds();

	auto MakeComponent = [this](UStaticMesh* Mesh, const TCHAR* Prefix)
	{
		const FName Name = MakeUniqueObjectName(this, UHierarchicalInstancedStaticMeshComponent::StaticClass(),
			*FString::Printf(TEXT("%s_%s"), Prefix, *Mesh->GetName()));
		UHierarchicalInstancedStaticMeshComponent* Component = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, Name, RF_Transactional);
		Component->CreationMethod = EComponentCreationMethod::Instance;
		Component->SetStaticMesh(Mesh);
		Component->SetGenerateOverlapEvents(false);
		Component->SetCanEverAffectNavigation(false);
		Component->SetupAttachment(Ground);
		return Component;
	};
	// Visible and purely visual (REQ-16 in HeliAce): nothing hits, blocks or navigates on decorations.
	auto MakeVisual = [&](int32 Index, const TCHAR* Prefix)
	{
		UStaticMesh* Mesh = Entries[Index].Mesh;
		UHierarchicalInstancedStaticMeshComponent* Component = MakeComponent(Mesh, Prefix);
		if (UMaterialInterface* Material = Entries[Index].Material)
		{
			for (int32 Slot = 0; Slot < Mesh->GetStaticMaterials().Num(); ++Slot)
			{
				Component->SetMaterial(Slot, Material);
			}
		}
		Component->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetCastShadow(Entries[Index].bCastShadow);
		Component->SetCullDistances(0, FMath::RoundToInt(Entries[Index].CullDistance));
		Component->RegisterComponent();
		AddInstanceComponent(Component);
		DecorationComponents.Add(Component);
		return Component;
	};

	// One HISM per mesh and material override, shared by entries that use the same pair.
	TMap<TPair<UStaticMesh*, UMaterialInterface*>, UHierarchicalInstancedStaticMeshComponent*> ByMesh;
	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		if (Entries[Index].Mesh == nullptr || PerEntry[Index].IsEmpty())
		{
			continue;
		}
		UHierarchicalInstancedStaticMeshComponent*& Component = ByMesh.FindOrAdd(MakeTuple(Entries[Index].Mesh.Get(), Entries[Index].Material.Get()));
		if (Component == nullptr)
		{
			Component = MakeVisual(Index, TEXT("Decor"));
		}
		Component->AddInstances(PerEntry[Index], /*bShouldReturnIndices*/ false, /*bWorldSpace*/ false);
	}

	GenerateTimings.InstancesMs += (FPlatformTime::Seconds() - InstancesStart) * 1000.0;

	UpdateGroundDarkness();
}

float ATerrainGenerator::LocalHeightAboveWater(const FVector2D& LocalXY) const
{
	return FTerrainHeightField::SampleGrid(Heights, GetGrid(), SizeX, SizeY, LocalXY.X, LocalXY.Y) - WaterLevel;
}

FVector ATerrainGenerator::LocalNormal(const FVector2D& LocalXY) const
{
	const float DX = FMath::Max(StepX, 1.f) * 0.5f;
	const float DY = FMath::Max(StepY, 1.f) * 0.5f;
	const float DHdX = (LocalHeightAboveWater(LocalXY + FVector2D(DX, 0.f)) - LocalHeightAboveWater(LocalXY - FVector2D(DX, 0.f))) / (2.f * DX);
	const float DHdY = (LocalHeightAboveWater(LocalXY + FVector2D(0.f, DY)) - LocalHeightAboveWater(LocalXY - FVector2D(0.f, DY))) / (2.f * DY);
	return FVector(-DHdX, -DHdY, 1.f).GetSafeNormal();
}

float ATerrainGenerator::GetWaterZ() const
{
	return GetActorLocation().Z + WaterLevel;
}

float ATerrainGenerator::GetHeightAt(FVector2D WorldXY) const
{
	const FVector Origin = GetActorLocation();
	if (TerrainType == ETerrainType::WaterOnly || NumX < 2 || NumY < 2 || Heights.Num() != NumX * NumY)
	{
		return Origin.Z + WaterLevel;
	}

	return Origin.Z + FTerrainHeightField::SampleGrid(Heights, GetGrid(), SizeX, SizeY, WorldXY.X - Origin.X, WorldXY.Y - Origin.Y);
}

float ATerrainGenerator::GetSurfaceHeightAt(FVector2D WorldXY) const
{
	const float GroundZ = GetHeightAt(WorldXY);
	return bWater ? FMath::Max(GroundZ, GetWaterZ()) : GroundZ;
}

bool ATerrainGenerator::IsLand(FVector2D WorldXY, float MinHeightAboveWater) const
{
	if (TerrainType == ETerrainType::WaterOnly)
	{
		return false;
	}
	return GetHeightAt(WorldXY) - GetWaterZ() >= MinHeightAboveWater;
}

