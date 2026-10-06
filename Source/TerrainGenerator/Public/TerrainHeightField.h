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

	float ShoreWidth = 0.2f;
	float SeaDepth = 1000.f;
	float CoastlineNoise = 0.6f;

	float WaterLevel = 0.f;

	TArray<FTerrainCorridor> Corridors;
};

/** Grid layout of a height field: NumX * NumY vertices, centred on the actor. */
struct TERRAINGENERATOR_API FTerrainGrid
{
	int32 NumX = 0;
	int32 NumY = 0;
	float StepX = 0.f;
	float StepY = 0.f;

	static FTerrainGrid Make(float SizeX, float SizeY, float CellSizeX, float CellSizeY);
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
};
