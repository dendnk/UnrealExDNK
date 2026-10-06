#include "TerrainGenerator.h"

#include "AI/NavigationSystemBase.h"
#include "Components/BoxComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// WaterFloor box thickness, cm. Its top face sits at WaterLevel.
	constexpr float WaterFloorThickness = 100.f;
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
	Params.ShoreWidth = ShoreWidth;
	Params.SeaDepth = SeaDepth;
	Params.CoastlineNoise = CoastlineNoise;
	Params.WaterLevel = WaterLevel;
	Params.Corridors = Corridors;
	return Params;
}

FTerrainGrid ATerrainGenerator::MakeGrid() const
{
	return FTerrainGrid::Make(SizeX, SizeY, CellSizeX > 0.f ? CellSizeX : CellSize, CellSizeY > 0.f ? CellSizeY : CellSize);
}

void ATerrainGenerator::Generate()
{
	BuildGroundMesh();
	UpdateWater();
	GenerateDecorations();
}

void ATerrainGenerator::BuildGroundMesh()
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
	FTerrainHeightField::Build(MakeHeightParams(), Grid, Heights);

	const float HalfX = SizeX * 0.5f;
	const float HalfY = SizeY * 0.5f;

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

	for (int32 Y = 0; Y < NumY; ++Y)
	{
		for (int32 X = 0; X < NumX; ++X)
		{
			const float PX = -HalfX + X * StepX;
			const float PY = -HalfY + Y * StepY;
			const float PZ = H(X, Y);

			// Central differences for smooth shading.
			const int32 X0 = FMath::Max(X - 1, 0), X1 = FMath::Min(X + 1, NumX - 1);
			const int32 Y0 = FMath::Max(Y - 1, 0), Y1 = FMath::Min(Y + 1, NumY - 1);
			const float DHdX = (H(X1, Y) - H(X0, Y)) / ((X1 - X0) * StepX);
			const float DHdY = (H(X, Y1) - H(X, Y0)) / ((Y1 - Y0) * StepY);
			const FVector Normal = FVector(-DHdX, -DHdY, 1.f).GetSafeNormal();

			Vertices.Add(FVector(PX, PY, PZ));
			Normals.Add(Normal);
			UV0.Add(FVector2D(PX, PY) / UVTileSize);
			// R = height above water [0..1], G = steepness [0..1]: blend sand/grass/rock in the material.
			const float HeightAlpha = MaxHeight > 0.f ? FMath::Clamp((PZ - WaterLevel) / MaxHeight, 0.f, 1.f) : 0.f;
			Colors.Add(FLinearColor(HeightAlpha, 1.f - Normal.Z, 0.f, 1.f));
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

	Ground->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, Colors, Tangents, /*bCreateCollision*/ true);
	if (GroundMaterial)
	{
		Ground->SetMaterial(0, GroundMaterial);
	}

	// Lets a dynamic navmesh rebuild over the new ground.
	FNavigationSystem::UpdateComponentData(*Ground);
}

void ATerrainGenerator::UpdateWater()
{
	// Engine plane mesh is 100x100 cm.
	const float WaterSize = FMath::Max(SizeX, SizeY) * WaterExtent;
	Water->SetRelativeLocation(FVector(0.f, 0.f, WaterLevel));
	Water->SetRelativeScale3D(FVector(WaterSize / 100.f, WaterSize / 100.f, 1.f));
	Water->SetVisibility(bWater);
	if (WaterMaterial)
	{
		Water->SetMaterial(0, WaterMaterial);
	}

	WaterFloor->SetRelativeLocation(FVector(0.f, 0.f, WaterLevel - WaterFloorThickness * 0.5f));
	WaterFloor->SetBoxExtent(FVector(WaterSize * 0.5f, WaterSize * 0.5f, WaterFloorThickness * 0.5f));
	WaterFloor->SetCollisionEnabled(bWater ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}

void ATerrainGenerator::ClearDecorations()
{
	for (UHierarchicalInstancedStaticMeshComponent* Component : DecorationComponents)
	{
		if (IsValid(Component))
		{
			RemoveInstanceComponent(Component);
			Component->DestroyComponent();
		}
	}
	DecorationComponents.Reset();
}

void ATerrainGenerator::GenerateDecorations()
{
	ClearDecorations();
	if (TerrainType == ETerrainType::WaterOnly || DecorationSet == nullptr || NumX < 2 || NumY < 2 || Heights.Num() != NumX * NumY)
	{
		return;
	}

	const TArray<FTerrainDecorationEntry>& Entries = DecorationSet->Entries;
	TArray<FTerrainDecorationMeshInfo> MeshInfo;
	MeshInfo.SetNum(Entries.Num());
	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		if (const UStaticMesh* Mesh = Entries[Index].Mesh)
		{
			const FBoxSphereBounds Bounds = Mesh->GetBounds();
			const FBox Box = Bounds.GetBox();
			// Reach from the pivot, not from the bounds centre: an off-centre pivot still can't sneak toward the lane.
			MeshInfo[Index].RadiusXY = FMath::Max(
				FVector2D(Box.Min.X, Box.Min.Y).Size(), FMath::Max(
				FVector2D(Box.Max.X, Box.Min.Y).Size(), FMath::Max(
				FVector2D(Box.Min.X, Box.Max.Y).Size(),
				FVector2D(Box.Max.X, Box.Max.Y).Size())));
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
	FTerrainDecorationScatter::Scatter(*DecorationSet, MeshInfo, Lane, GroundQuery, Stream, PerEntry);

	// One HISM per mesh, shared by entries that use the same mesh.
	TMap<UStaticMesh*, UHierarchicalInstancedStaticMeshComponent*> ByMesh;
	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		UStaticMesh* Mesh = Entries[Index].Mesh;
		if (Mesh == nullptr || PerEntry[Index].IsEmpty())
		{
			continue;
		}

		UHierarchicalInstancedStaticMeshComponent*& Component = ByMesh.FindOrAdd(Mesh);
		if (Component == nullptr)
		{
			const FName Name = MakeUniqueObjectName(this, UHierarchicalInstancedStaticMeshComponent::StaticClass(),
				*FString::Printf(TEXT("Decor_%s"), *Mesh->GetName()));
			Component = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, Name, RF_Transactional);
			Component->CreationMethod = EComponentCreationMethod::Instance;
			Component->SetStaticMesh(Mesh);
			// Purely visual (REQ-16 in HeliAce): nothing hits, blocks or navigates on decorations.
			Component->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
			Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Component->SetGenerateOverlapEvents(false);
			Component->SetCanEverAffectNavigation(false);
			Component->SetCastShadow(Entries[Index].bCastShadow);
			Component->SetCullDistances(0, FMath::RoundToInt(Entries[Index].CullDistance));
			Component->SetupAttachment(Ground);
			Component->RegisterComponent();
			AddInstanceComponent(Component);
			DecorationComponents.Add(Component);
		}
		Component->AddInstances(PerEntry[Index], /*bShouldReturnIndices*/ false, /*bWorldSpace*/ false);
	}
}

float ATerrainGenerator::LocalHeightAboveWater(const FVector2D& LocalXY) const
{
	const FTerrainGrid Grid{ NumX, NumY, StepX, StepY };
	return FTerrainHeightField::SampleGrid(Heights, Grid, SizeX, SizeY, LocalXY.X, LocalXY.Y) - WaterLevel;
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

	const FTerrainGrid Grid{ NumX, NumY, StepX, StepY };
	return Origin.Z + FTerrainHeightField::SampleGrid(Heights, Grid, SizeX, SizeY, WorldXY.X - Origin.X, WorldXY.Y - Origin.Y);
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

bool ATerrainGenerator::FindSpawnPoint(FVector& OutLocation, float MinHeightAboveWater, float MaxSlopeDegrees, int32 MaxAttempts)
{
	const FVector Origin = GetActorLocation();
	const float MinNormalZ = FMath::Cos(FMath::DegreesToRadians(MaxSlopeDegrees));
	const float HalfX = SizeX * 0.5f;
	const float HalfY = SizeY * 0.5f;
	const float D = FMath::Max(StepX, 1.f) * 0.5f;

	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		const FVector2D P(Origin.X + SpawnStream.FRandRange(-HalfX, HalfX), Origin.Y + SpawnStream.FRandRange(-HalfY, HalfY));
		if (!IsLand(P, MinHeightAboveWater))
		{
			continue;
		}

		const float DHdX = (GetHeightAt(P + FVector2D(D, 0.f)) - GetHeightAt(P - FVector2D(D, 0.f))) / (2.f * D);
		const float DHdY = (GetHeightAt(P + FVector2D(0.f, D)) - GetHeightAt(P - FVector2D(0.f, D))) / (2.f * D);
		if (FVector(-DHdX, -DHdY, 1.f).GetSafeNormal().Z < MinNormalZ)
		{
			continue;
		}

		OutLocation = FVector(P, GetHeightAt(P));
		return true;
	}
	return false;
}
