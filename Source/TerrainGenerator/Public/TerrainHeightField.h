#pragma once

#include "CoreMinimal.h"
#include "TerrainTypes.h"

/** Everything that shapes the height field. Mirrors ATerrainGenerator's properties; all values actor-local, cm. */
struct TERRAINGENERATOR_API FTerrainHeightParams
{
	ETerrainType Type = ETerrainType::Island;

	float SizeX = 200000.f;
	float SizeY = 200000.f;
	float MaxHeight = 4000.f;
	float MinLandHeight = 200.f;

	int32 Seed = 1337;
	float NoiseScale = 0.00004f;
	int32 Octaves = 4;
	float Persistence = 0.45f;
	float Lacunarity = 2.f;
	float Flatness = 1.6f;

	/** Share of ridged noise mixed into the high ground, 0..1 (0 = plain fBm). Corridor profiles ignore it. */
	float RidgeWeight = 0.f;
	/** Normalised height (0..1, before Flatness) where the ridges start to blend in. */
	float RidgeStart = 0.35f;

	float ShoreWidth = 0.2f;
	float SeaDepth = 1000.f;
	float CoastlineNoise = 0.6f;

	float WaterLevel = 0.f;

	TArray<FTerrainCorridor> Corridors;
};

/**
 * Grid layout of a height field: NumX * NumY vertices, centred on the actor. Columns are StepX apart, except that
 * from column FineFrom on they are FineStepX apart: finer ground toward the +X end (an island's rounded end).
 */
struct TERRAINGENERATOR_API FTerrainGrid
{
	int32 NumX = 0;
	int32 NumY = 0;
	float StepX = 0.f;
	float StepY = 0.f;
	/** First column of the fine end; INDEX_NONE = every column StepX apart. */
	int32 FineFrom = INDEX_NONE;
	float FineStepX = 0.f;

	/**
	 * FineLength > 0 with FineCellSizeX > 0: the columns over the last FineLength of the grid (starting at the first
	 * regular column inside it, so the columns before it sit exactly where a regular grid has them) are about
	 * FineCellSizeX apart.
	 */
	static FTerrainGrid Make(float SizeX, float SizeY, float CellSizeX, float CellSizeY, float FineLength = 0.f, float FineCellSizeX = 0.f);

	bool HasFineEnd() const { return FineFrom != INDEX_NONE && FineFrom < NumX - 1; }

	/** Local X of column X. */
	float ColumnX(int32 X, float SizeX) const;

	/** Distance from column X to column X + 1. */
	float StepAfter(int32 X) const { return HasFineEnd() && X >= FineFrom ? FineStepX : StepX; }

	/** Fractional column at local X, clamped to [0, NumX - 1]. */
	float ColumnAt(float LocalX, float SizeX) const;
};

/** Pure height maths: no world, no components, so it can be tested headless. */
struct TERRAINGENERATOR_API FTerrainHeightField
{
	/**
	 * Fills OutHeights (row-major, Y * NumX + X) with heights relative to the actor.
	 * Order: raw fBm -> island mask (Island only) -> corridors (clamp, slope limit, side cap).
	 * WaterOnly gives an empty array.
	 */
	static void Build(const FTerrainHeightParams& Params, const FTerrainGrid& Grid, TArray<float>& OutHeights);

	/** Height at a local XY point, interpolated on the same triangle split the mesh uses (diagonal V1 -> V2). */
	static float SampleGrid(const TArray<float>& Heights, const FTerrainGrid& Grid, float SizeX, float SizeY, float LocalX, float LocalY);

	/** Noise offset derived from the seed, exactly as the generator has always done it. */
	static FVector2D MakeNoiseOffset(int32 Seed);

	/** Height before corridors. bApplyMask = false skips the island mask (Continent, or corridors overriding the shore). */
	static float SampleBase(const FTerrainHeightParams& Params, const FVector2D& NoiseOffset, float X, float Y, bool bApplyMask);

	/**
	 * Baked ambient occlusion per vertex (same layout as Heights), 0 = open .. 1 = enclosed: how far each vertex lies below
	 * the mean ground on two rings around it (Radius and Radius / 2, 8 samples each), relative to Depth.
	 * Valleys and slope bottoms get > 0; flat ground, slopes and ridges 0. Radius or Depth <= 0 gives all zeros.
	 */
	static void ComputeOcclusion(const TArray<float>& Heights, const FTerrainGrid& Grid, float SizeX, float SizeY, float Radius, float Depth,
		TArray<float>& OutOcclusion);

	/**
	 * Path mask of a corridor at a local XY point, 0..1: a linear tent across Y (1 on the centre line, 0 at HalfWidth),
	 * faded out over the corridor's Falloff beyond StartX..EndX.
	 */
	static float PathWeight(const FTerrainCorridor& Corridor, float HalfWidth, float LocalX, float LocalY);

	/** Darkens a soft round patch: InOutDarkness[v] = max(itself, Strength * (1 - d / Radius)^2) for vertices within Radius of Center. */
	static void AddContactDarkening(const FTerrainGrid& Grid, float SizeX, float SizeY, const FVector2D& Center, float Radius, float Strength,
		TArray<float>& InOutDarkness);
};
