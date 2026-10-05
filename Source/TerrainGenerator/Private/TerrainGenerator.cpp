#include "TerrainGenerator.h"

#include "AI/NavigationSystemBase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

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
	if (!Ground)
	{
		return;
	}

	Water->SetupAttachment(Ground);
	// Switch to a query-only profile if projectiles should detect water hits (splashes).
	Water->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Water->SetCanEverAffectNavigation(false);
	Water->CastShadow = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		Water->SetStaticMesh(PlaneMesh.Object);
	}
}

void ATerrainGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Generate();
}

void ATerrainGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (bRandomSeedOnBeginPlay)
	{
		Seed = FMath::Rand();
		Generate();
	}
	else if (Heights.Num() != NumX * NumY || NumX < 2)
	{
		Generate();
	}
	else
	{
		SpawnStream.Initialize(Seed);
	}
}

float ATerrainGenerator::SampleHeight(float X, float Y) const
{
	// Fractal Brownian motion: sum of Perlin octaves -> smooth rolling hills.
	float Amplitude = 1.f;
	float Frequency = NoiseScale;
	float Sum = 0.f;
	float Norm = 0.f;
	for (int32 Octave = 0; Octave < Octaves; ++Octave)
	{
		Sum += Amplitude * FMath::PerlinNoise2D(FVector2D(X, Y) * Frequency + NoiseOffset);
		Norm += Amplitude;
		Amplitude *= Persistence;
		Frequency *= Lacunarity;
	}
	// Perlin output rarely reaches +-1, so stretch it a bit before mapping to [0,1].
	float H = FMath::Clamp((Sum / Norm) * 0.7f + 0.5f, 0.f, 1.f);
	H = FMath::Pow(H, Flatness);

	const float LandZ = WaterLevel + MinLandHeight + H * (MaxHeight - MinLandHeight);

	// Island mask: 0 at the border (under water), 1 in the interior.
	const float HalfX = SizeX * 0.5f;
	const float HalfY = SizeY * 0.5f;
	const float EdgeX = (HalfX - FMath::Abs(X)) / (HalfX * ShoreWidth);
	const float EdgeY = (HalfY - FMath::Abs(Y)) / (HalfY * ShoreWidth);
	float Edge = FMath::Min(EdgeX, EdgeY);

	// Only ever pushes the shore inward, so the mesh border always stays under water.
	const float Coast = FMath::PerlinNoise2D(FVector2D(X, Y) * (NoiseScale * 3.f) - NoiseOffset);
	Edge -= CoastlineNoise * (Coast * 0.5f + 0.5f);

	const float Mask = FMath::SmoothStep(0.f, 1.f, Edge);
	return FMath::Lerp(WaterLevel - SeaDepth, LandZ, Mask);
}

void ATerrainGenerator::Generate()
{
	NumX = FMath::Max(2, FMath::CeilToInt(SizeX / CellSize) + 1);
	NumY = FMath::Max(2, FMath::CeilToInt(SizeY / CellSize) + 1);
	StepX = SizeX / (NumX - 1);
	StepY = SizeY / (NumY - 1);

	FRandomStream Stream(Seed);
	NoiseOffset = FVector2D(Stream.FRandRange(-1000.f, 1000.f), Stream.FRandRange(-1000.f, 1000.f));
	SpawnStream.Initialize(Seed);

	const float HalfX = SizeX * 0.5f;
	const float HalfY = SizeY * 0.5f;

	Heights.SetNumUninitialized(NumX * NumY);
	for (int32 Y = 0; Y < NumY; ++Y)
	{
		for (int32 X = 0; X < NumX; ++X)
		{
			Heights[Y * NumX + X] = SampleHeight(-HalfX + X * StepX, -HalfY + Y * StepY);
		}
	}

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

	// Two triangles per cell, wound so faces point up (+Z). Diagonal runs V1 -> V2.
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

	Ground->ClearAllMeshSections();
	Ground->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, Colors, Tangents, /*bCreateCollision*/ true);
	if (GroundMaterial)
	{
		Ground->SetMaterial(0, GroundMaterial);
	}

	// Engine plane mesh is 100x100 cm.
	const float WaterSize = FMath::Max(SizeX, SizeY) * WaterExtent;
	Water->SetRelativeLocation(FVector(0.f, 0.f, WaterLevel));
	Water->SetRelativeScale3D(FVector(WaterSize / 100.f, WaterSize / 100.f, 1.f));
	if (WaterMaterial)
	{
		Water->SetMaterial(0, WaterMaterial);
	}

	// Lets a dynamic navmesh rebuild over the new ground.
	FNavigationSystem::UpdateComponentData(*Ground);
}

float ATerrainGenerator::GetHeightAt(FVector2D WorldXY) const
{
	const FVector Origin = GetActorLocation();
	if (NumX < 2 || NumY < 2 || Heights.Num() != NumX * NumY)
	{
		return Origin.Z + WaterLevel;
	}

	const float FX = FMath::Clamp((WorldXY.X - Origin.X + SizeX * 0.5f) / StepX, 0.f, float(NumX - 1));
	const float FY = FMath::Clamp((WorldXY.Y - Origin.Y + SizeY * 0.5f) / StepY, 0.f, float(NumY - 1));
	const int32 X0 = FMath::Min(FMath::FloorToInt(FX), NumX - 2);
	const int32 Y0 = FMath::Min(FMath::FloorToInt(FY), NumY - 2);
	const float TX = FX - X0;
	const float TY = FY - Y0;

	const float H00 = Heights[Y0 * NumX + X0];
	const float H10 = Heights[Y0 * NumX + X0 + 1];
	const float H01 = Heights[(Y0 + 1) * NumX + X0];
	const float H11 = Heights[(Y0 + 1) * NumX + X0 + 1];

	// Interpolate on the same triangle the mesh uses, so actors sit exactly on the surface.
	const float Z = (TX + TY <= 1.f)
		? H00 + TX * (H10 - H00) + TY * (H01 - H00)
		: H11 + (1.f - TX) * (H01 - H11) + (1.f - TY) * (H10 - H11);

	return Origin.Z + Z;
}

bool ATerrainGenerator::IsLand(FVector2D WorldXY, float MinHeightAboveWater) const
{
	return GetHeightAt(WorldXY) - (GetActorLocation().Z + WaterLevel) >= MinHeightAboveWater;
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
