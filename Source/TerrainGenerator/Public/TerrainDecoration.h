#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Math/RandomStream.h"
#include "TerrainTypes.h"
#include "TerrainDecoration.generated.h"

class UStaticMesh;

/** One kind of decoration. Always visual only: there is deliberately no collision option. */
USTRUCT(BlueprintType)
struct TERRAINGENERATOR_API FTerrainDecorationEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration")
	TObjectPtr<UStaticMesh> Mesh;

	/** Relative chance of this entry when a band picks one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration", meta = (ClampMin = "0"))
	float Weight = 1.f;

	/** Uniform scale range (min, max). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration")
	FVector2f ScaleRange = FVector2f(0.8f, 1.2f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration")
	bool bRandomYaw = true;

	/** 0 = stand upright, 1 = fully follow the ground normal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration", meta = (ClampMin = "0", ClampMax = "1"))
	float AlignToNormal = 0.f;

	/** Allowed ground height above the water level (min, max), cm. With water the min is never below 0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration")
	FVector2f HeightAboveWater = FVector2f(50.f, 100000.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration", meta = (ClampMin = "0", ClampMax = "90"))
	float MaxSlopeDeg = 30.f;

	/** No other instance of this entry closer than this, cm. Against other entries, the smaller of the two spacings applies. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration", meta = (ClampMin = "0"))
	float MinSpacing = 800.f;

	/** Pushes the mesh into the ground so slopes don't show a gap, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration")
	float SinkDepth = 20.f;

	/** Off by default for mobile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration|Rendering")
	bool bCastShadow = false;

	/** Instances fade out at this distance, cm. 0 = never culled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration|Rendering", meta = (ClampMin = "0"))
	float CullDistance = 0.f;
};

/** A strip parallel to the lane, on the band side, MinDistance..MaxDistance from the lane centre line. */
USTRUCT(BlueprintType)
struct TERRAINGENERATOR_API FTerrainDecorationBand
{
	GENERATED_BODY()

	/** Clamped to the set's MinLaneDistance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Band", meta = (ClampMin = "0"))
	float MinDistance = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Band", meta = (ClampMin = "0"))
	float MaxDistance = 20000.f;

	/** Candidate samples per 10 000 m² (one hectare). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Band", meta = (ClampMin = "0.01"))
	float Density = 4.f;

	/** Entries this band may place (indices into the set's Entries). Empty = all. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Band")
	TArray<int32> EntryIndices;

	/** Tallest scaled mesh allowed in this band, cm. 0 = no cap (e.g. keep near bands low). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Band", meta = (ClampMin = "0"))
	float MaxMeshHeight = 0.f;
};

/** Seeded decoration layout along a terrain corridor. Placed on one side only. */
UCLASS(BlueprintType)
class TERRAINGENERATOR_API UTerrainDecorationSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration")
	TArray<FTerrainDecorationEntry> Entries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration")
	TArray<FTerrainDecorationBand> Bands;

	/** No instance's bounds come closer than this to the corridor centre line, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration", meta = (ClampMin = "0"))
	float MinLaneDistance = 5000.f;

	/** Which side of the corridor the bands are on: -1 = -Y, +1 = +Y. Never both. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration", meta = (ClampMin = "-1", ClampMax = "1"))
	int32 BandSideSign = -1;
};

/** Mesh measurements the scatter needs, one per entry. */
struct FTerrainDecorationMeshInfo
{
	/** Horizontal reach of the unscaled mesh from its pivot, cm. */
	float RadiusXY = 0.f;
	/** Top of the unscaled mesh above its pivot, cm. */
	float Height = 0.f;
};

/** Ground queries in actor-local space. */
struct FTerrainDecorationGround
{
	/** Ground height above the water level at a local XY point. */
	TFunction<float(const FVector2D&)> HeightAboveWater;
	/** Ground normal at a local XY point. */
	TFunction<FVector(const FVector2D&)> Normal;
	/** Local Z of the water level. */
	float WaterZ = 0.f;
	/** Whether the terrain has water; without it, entries may go below the water level (dry valleys). */
	bool bHasWater = true;
};

/** Pure scatter: no world or components, so it can be tested headless. */
struct TERRAINGENERATOR_API FTerrainDecorationScatter
{
	/**
	 * Jittered grid per band over [Lane.StartX, Lane.EndX], weighted entry pick. A sample is rejected if it is wet or
	 * outside the entry's height range, too steep, too close to an accepted instance (MinSpacing; see the entry), too tall for the
	 * band, or if its scaled bounds reach inside MinLaneDistance of the lane centre line.
	 * Draws a fixed number of random values per sample, so the layout depends only on the stream and the set.
	 * OutPerEntry gets one array of actor-local transforms per entry.
	 */
	static void Scatter(const UTerrainDecorationSet& Set, TConstArrayView<FTerrainDecorationMeshInfo> MeshInfo,
		const FTerrainCorridor& Lane, const FTerrainDecorationGround& Ground, FRandomStream& Stream,
		TArray<TArray<FTransform>>& OutPerEntry);

	/** Jittered-grid cell size for a density per hectare, cm. */
	static float CellSizeForDensity(float DensityPerHectare);
};
