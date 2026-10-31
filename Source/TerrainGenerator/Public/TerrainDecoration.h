#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Math/RandomStream.h"
#include "TerrainTypes.h"
#include "TerrainDecoration.generated.h"

class UMaterialInterface;
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

	/**
	 * Ground the whole footprint (e.g. mountains): the base goes to the lowest ground under the mesh's bounds, or to the
	 * water level where they overhang the sea, so a wide mesh never hangs in the air. The height range then only needs
	 * some of the footprint on land, and the slope limit is not checked.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration")
	bool bGroundFootprint = false;

	/**
	 * Part of the scaled footprint radius the mesh stands on (0..1): the base sinks until the mesh's base plane, tilted
	 * with it, is at or below the ground across that disc, so no side floats on a slope. 0 = the centre point only.
	 * Ignored with bGroundFootprint.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration", meta = (ClampMin = "0", ClampMax = "1"))
	float ContactRadiusFraction = 0.f;

	/** Off by default for mobile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration|Rendering")
	bool bCastShadow = false;

	/** Instances fade out at this distance, cm. 0 = never culled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration|Rendering", meta = (ClampMin = "0"))
	float CullDistance = 0.f;

	/** Replaces every material slot of the mesh for this entry (e.g. a recoloured palette). None = the mesh's own materials. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration|Rendering")
	TObjectPtr<UMaterialInterface> Material;
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

	/** Which side of the corridor this band is on: -1 = -Y, +1 = +Y, 0 = the set's BandSideSign. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Band", meta = (ClampMin = "-1", ClampMax = "1"))
	int32 Side = 0;

	/** Reject instances whose bounds reach past MinDistance (toward the corridor). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Band")
	bool bContainInnerEdge = false;

	/** Reject instances whose bounds reach past MaxDistance (away from the corridor). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Band")
	bool bContainOuterEdge = false;

	/**
	 * Place each instance by the edge of its bounds toward the corridor instead of by its centre: that edge lands
	 * MinDistance..MaxDistance out and the rest of the mesh extends away from the corridor, however wide it is.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Band")
	bool bPlaceByInnerEdge = false;
};

/**
 * Rock pieces on the decoration corridor's cliffs (HeliAce specs/<status>-terrain-cliff-drops REQ-11): boulders at the
 * toe, rocks half sunk into the face and a few on the lip, all inside the cliff's enemy keep-out (the corridor's
 * CliffMarginLow / CliffMarginHigh). Unlike the other decorations, rocks near the lane are solid: they block like the
 * ground, through an invisible copy shrunk to CollisionScale, so the collision sits inside the visible rock.
 */
USTRUCT(BlueprintType)
struct TERRAINGENERATOR_API FTerrainCliffRockSettings
{
	GENERATED_BODY()

	/** Entries (indices into the set's Entries) to build the rocks from: mesh, weight, material, shadow and cull distance. Empty = off. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Rocks")
	TArray<int32> EntryIndices;

	/** Distance between neighbouring rocks along a cliff (across the island, Y), cm (min, max). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Rocks")
	FVector2f Spacing = FVector2f(300.f, 600.f);

	/** Relative shares of toe, face and lip rocks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Rocks", meta = (ClampMin = "0"))
	float ToeWeight = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Rocks", meta = (ClampMin = "0"))
	float FaceWeight = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Rocks", meta = (ClampMin = "0"))
	float LipWeight = 0.2f;

	/** Width of a rock (its scaled bounds across XY), cm (min, max), per kind. A lip rock is also kept small enough for CliffMarginHigh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Rocks")
	FVector2f ToeSize = FVector2f(150.f, 400.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Rocks")
	FVector2f FaceSize = FVector2f(150.f, 350.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Rocks")
	FVector2f LipSize = FVector2f(80.f, 200.f);

	/** How far the rocks run along each cliff from the corridor centre line toward -Y and +Y, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Rocks", meta = (ClampMin = "0"))
	float ReachMinusY = 8000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Rocks", meta = (ClampMin = "0"))
	float ReachPlusY = 8000.f;

	/** Rocks whose bounds come within this of the corridor centre line (Y) are solid; farther ones only show (nothing flies there). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Rocks", meta = (ClampMin = "0"))
	float SolidHalfWidth = 3000.f;

	/** Size of the collision against the visible rock, about its bounds centre (0.9 = 10% inside the silhouette). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Rocks", meta = (ClampMin = "0.1", ClampMax = "1"))
	float CollisionScale = 0.9f;

	/** Random tilt of each rock, degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Rocks", meta = (ClampMin = "0", ClampMax = "45"))
	float MaxTiltDeg = 20.f;
};

/** Seeded decoration layout along a terrain corridor, in bands on either side of it. */
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

	/** Default side for bands that don't set their own Side: -1 = -Y, +1 = +Y. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration", meta = (ClampMin = "-1", ClampMax = "1"))
	int32 BandSideSign = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoration")
	FTerrainCliffRockSettings CliffRocks;
};

/** Where a cliff rock sits on its cliff. */
enum class ETerrainCliffRockKind : uint8
{
	Toe,
	Face,
	Lip
};

/** One cliff rock: its entry, actor-local transform, and whether it blocks (near the lane). */
struct FTerrainCliffRock
{
	int32 EntryIndex = INDEX_NONE;
	FTransform Transform;
	ETerrainCliffRockKind Kind = ETerrainCliffRockKind::Toe;
	bool bSolid = false;
	/** Horizontal reach of the scaled rock from its bounds centre (X, Y), cm. */
	float Radius = 0.f;
	/** Bounds centre, actor-local. */
	FVector Center = FVector::ZeroVector;
};

/** Mesh measurements the scatter needs, one per entry. */
struct FTerrainDecorationMeshInfo
{
	/** Horizontal reach of the unscaled mesh from its bounds centre (CenterXY), cm. */
	float RadiusXY = 0.f;
	/** Top of the unscaled mesh above its pivot, cm. */
	float Height = 0.f;
	/**
	 * Bounds centre of the unscaled mesh in its own XY, cm. The scatter puts this point, not the pivot, on the sample,
	 * so meshes imported with a far-off pivot (e.g. a pack's scene position) still land where they were placed.
	 */
	FVector2D CenterXY = FVector2D::ZeroVector;
	/** Bounds centre Z and half height of the unscaled mesh, cm (cliff rocks are placed by their bounds centre). */
	float CenterZ = 0.f;
	float ExtentZ = 0.f;
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

	/**
	 * Rocks along every cliff of Lane (Set.CliffRocks), seeded per cliff from Seed. Every rock's bounds stay inside its
	 * cliff's keep-out on X ([zone edge - margin, zone edge + margin]), none stands in the water, and each sits on its
	 * lowest ground (toe, lip) or half sunk into the face, so nothing floats.
	 */
	static void ScatterCliffRocks(const UTerrainDecorationSet& Set, TConstArrayView<FTerrainDecorationMeshInfo> MeshInfo,
		const FTerrainCorridor& Lane, const FTerrainDecorationGround& Ground, int32 Seed, TArray<FTerrainCliffRock>& OutRocks);

	/** Transform of a rock's collision copy: scaled by CollisionScale about the mesh's bounds centre (MeshCenter, unscaled local). */
	static FTransform CollisionTransform(const FTransform& Visual, const FVector& MeshCenter, float CollisionScale);
};
