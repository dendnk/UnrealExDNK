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
 * InsertColumns adds columns at given X (cliff edges); from then on the columns are an explicit list.
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
	/** Explicit column positions, distance from the -X edge, ascending; empty = the regular layout above. */
	TArray<float> Columns;
	/** Explicit row positions, distance from the -Y edge, ascending; empty = every row StepY apart (see ThinRows). */
	TArray<float> Rows;

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
	float StepAfter(int32 X) const
	{
		return !Columns.IsEmpty() ? Columns[X + 1] - Columns[X] : HasFineEnd() && X >= FineFrom ? FineStepX : StepX;
	}

	/**
	 * Adds a column at each local X in LocalXs (inside the grid) and drops the other columns closer than MinGap to one.
	 * LocalXs are cliff faces' edge pairs that don't overlap (start, end, start, end, ...); columns between a pair go too,
	 * so each face is one cell. FineFrom keeps pointing at the same position.
	 */
	void InsertColumns(TConstArrayView<float> LocalXs, float SizeX, float MinGap = 100.f);

	/** Index of the column at local X (within Tolerance), INDEX_NONE if there is none. */
	int32 FindColumn(float LocalX, float SizeX, float Tolerance = 1.f) const;

	/** Fractional column at local X, clamped to [0, NumX - 1]. */
	float ColumnAt(float LocalX, float SizeX) const;

	/** Local Y of row Y. */
	float RowY(int32 Y, float SizeY) const
	{
		return -SizeY * 0.5f + (!Rows.IsEmpty() ? Rows[Y] : Y * StepY);
	}

	/** Fractional row at local Y, clamped to [0, NumY - 1]. */
	float RowAt(float LocalY, float SizeY) const;

	/**
	 * Keeps every row with local Y in [FineFromY, FineToY] (and one row past each end, so the band's edge cells stay
	 * StepY wide), plus every Every-th row and both edges elsewhere: fewer triangles on background ground. The kept rows
	 * sit exactly where the regular grid has them, so ground inside the band is unchanged. StepY stays the fine spacing.
	 * Rows at KeepYs (local Y, nearest row) stay too: lines a check samples exactly, like a land-width edge.
	 */
	void ThinRows(float FineFromY, float FineToY, int32 Every, float SizeY, TConstArrayView<float> KeepYs = {});
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

	/** SampleGrid with the fractional column (Grid.ColumnAt) already known. The grid must be valid for Heights. */
	static float SampleGridAtColumn(const TArray<float>& Heights, const FTerrainGrid& Grid, float SizeY, float FX, float LocalY);

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

	/** Local X of both face edges of every cliff of every corridor: where ATerrainGenerator::MakeGrid inserts columns. */
	static TArray<float> CliffColumnXs(TConstArrayView<FTerrainCorridor> Corridors);

	/** Darkens a soft round patch: InOutDarkness[v] = max(itself, Strength * (1 - d / Radius)^2) for vertices within Radius of Center. */
	static void AddContactDarkening(const FTerrainGrid& Grid, float SizeX, float SizeY, const FVector2D& Center, float Radius, float Strength,
		TArray<float>& InOutDarkness);
};
