#include "TerrainDecoration.h"

namespace
{
	struct FPlacedDecoration
	{
		FVector2D Position;
		float Spacing = 0.f;
		int32 EntryIndex = INDEX_NONE;
	};

	/**
	 * Same entry: its own MinSpacing. Different entries: the smaller of the two, so one widely spaced entry (a mountain)
	 * keeps its copies apart without clearing everything else around it.
	 */
	float RequiredSpacing(float Spacing, int32 EntryIndex, const FPlacedDecoration& Other)
	{
		return EntryIndex == Other.EntryIndex ? Spacing : FMath::Min(Spacing, Other.Spacing);
	}

	/** Spatial hash with cells at least as large as the largest spacing, so a 3x3 lookup finds every conflict. */
	struct FSpacingGrid
	{
		float CellSize = 100.f;
		TMap<FIntPoint, TArray<FPlacedDecoration>> Cells;

		FIntPoint KeyOf(const FVector2D& P) const
		{
			return FIntPoint(FMath::FloorToInt(P.X / CellSize), FMath::FloorToInt(P.Y / CellSize));
		}

		bool IsFree(const FVector2D& P, float Spacing, int32 EntryIndex) const
		{
			const FIntPoint Key = KeyOf(P);
			for (int32 DY = -1; DY <= 1; ++DY)
			{
				for (int32 DX = -1; DX <= 1; ++DX)
				{
					if (const TArray<FPlacedDecoration>* Cell = Cells.Find(Key + FIntPoint(DX, DY)))
					{
						for (const FPlacedDecoration& Other : *Cell)
						{
							const float Required = RequiredSpacing(Spacing, EntryIndex, Other);
							if (FVector2D::DistSquared(P, Other.Position) < Required * Required)
							{
								return false;
							}
						}
					}
				}
			}
			return true;
		}

		void Add(const FVector2D& P, float Spacing, int32 EntryIndex)
		{
			Cells.FindOrAdd(KeyOf(P)).Add({ P, Spacing, EntryIndex });
		}
	};

	// A band this dense would hang the editor; it's a data mistake, not a layout.
	constexpr int64 MaxSamplesPerBand = 1000000;
}

float FTerrainDecorationScatter::CellSizeForDensity(float DensityPerHectare)
{
	// 1 ha = 10 000 m² = 1e8 cm², one sample per cell.
	return 10000.f / FMath::Sqrt(FMath::Max(DensityPerHectare, 0.01f));
}

void FTerrainDecorationScatter::Scatter(const UTerrainDecorationSet& Set, TConstArrayView<FTerrainDecorationMeshInfo> MeshInfo,
	const FTerrainCorridor& Lane, const FTerrainDecorationGround& Ground, FRandomStream& Stream,
	TArray<TArray<FTransform>>& OutPerEntry)
{
	OutPerEntry.Reset();
	OutPerEntry.SetNum(Set.Entries.Num());
	if (Set.Entries.IsEmpty() || MeshInfo.Num() != Set.Entries.Num() || !Ground.HeightAboveWater || !Ground.Normal)
	{
		return;
	}

	const float Side = Set.BandSideSign >= 0 ? 1.f : -1.f;
	const float StartX = FMath::Min(Lane.StartX, Lane.EndX);
	const float EndX = FMath::Max(Lane.StartX, Lane.EndX);

	FSpacingGrid Spacing;
	for (const FTerrainDecorationEntry& Entry : Set.Entries)
	{
		Spacing.CellSize = FMath::Max(Spacing.CellSize, Entry.MinSpacing);
	}

	const FVector Up = FVector::UpVector;

	for (const FTerrainDecorationBand& Band : Set.Bands)
	{
		const float MinDistance = FMath::Max(Band.MinDistance, Set.MinLaneDistance);
		const float MaxDistance = Band.MaxDistance;
		if (MaxDistance <= MinDistance || EndX <= StartX)
		{
			continue;
		}

		TArray<int32> Candidates;
		float TotalWeight = 0.f;
		auto AddCandidate = [&](int32 Index)
		{
			if (Set.Entries.IsValidIndex(Index) && Set.Entries[Index].Weight > 0.f)
			{
				Candidates.Add(Index);
				TotalWeight += Set.Entries[Index].Weight;
			}
		};
		if (Band.EntryIndices.IsEmpty())
		{
			for (int32 Index = 0; Index < Set.Entries.Num(); ++Index)
			{
				AddCandidate(Index);
			}
		}
		else
		{
			for (const int32 Index : Band.EntryIndices)
			{
				AddCandidate(Index);
			}
		}
		if (Candidates.IsEmpty())
		{
			continue;
		}

		const float Cell = CellSizeForDensity(Band.Density);
		const int32 NumCX = FMath::CeilToInt((EndX - StartX) / Cell);
		const int32 NumCY = FMath::CeilToInt((MaxDistance - MinDistance) / Cell);
		if (int64(NumCX) * int64(NumCY) > MaxSamplesPerBand)
		{
			ensureMsgf(false, TEXT("Terrain decoration band too dense (%d x %d samples); skipped."), NumCX, NumCY);
			continue;
		}

		for (int32 CY = 0; CY < NumCY; ++CY)
		{
			for (int32 CX = 0; CX < NumCX; ++CX)
			{
				// Always five draws per sample, so a rejection never shifts the rest of the layout.
				const float JitterX = Stream.FRand();
				const float JitterD = Stream.FRand();
				const float Pick = Stream.FRand() * TotalWeight;
				const float ScaleAlpha = Stream.FRand();
				const float YawAlpha = Stream.FRand();

				const float X = StartX + (CX + JitterX) * Cell;
				const float Distance = MinDistance + (CY + JitterD) * Cell;
				if (X > EndX || Distance > MaxDistance)
				{
					continue;
				}

				int32 EntryIndex = Candidates.Last();
				float Accumulated = 0.f;
				for (const int32 Candidate : Candidates)
				{
					Accumulated += Set.Entries[Candidate].Weight;
					if (Pick < Accumulated)
					{
						EntryIndex = Candidate;
						break;
					}
				}
				const FTerrainDecorationEntry& Entry = Set.Entries[EntryIndex];
				const FTerrainDecorationMeshInfo& Info = MeshInfo[EntryIndex];

				const float Scale = FMath::Lerp(Entry.ScaleRange.X, Entry.ScaleRange.Y, ScaleAlpha);

				// Bounds, not pivot: a wide canopy must not reach toward the lane.
				if (Distance - Info.RadiusXY * Scale < Set.MinLaneDistance)
				{
					continue;
				}
				if (Band.MaxMeshHeight > 0.f && Info.Height * Scale > Band.MaxMeshHeight)
				{
					continue;
				}

				const FVector2D P(X, Lane.CenterY + Side * Distance);
				const float Height = Ground.HeightAboveWater(P);
				const float MinHeight = Ground.bHasWater ? FMath::Max(0.f, Entry.HeightAboveWater.X) : Entry.HeightAboveWater.X;
				if (Height < MinHeight || Height > Entry.HeightAboveWater.Y)
				{
					continue;
				}

				const FVector Normal = Ground.Normal(P);
				if (Normal.Z < FMath::Cos(FMath::DegreesToRadians(Entry.MaxSlopeDeg)))
				{
					continue;
				}

				if (!Spacing.IsFree(P, Entry.MinSpacing, EntryIndex))
				{
					continue;
				}
				Spacing.Add(P, Entry.MinSpacing, EntryIndex);

				const FQuat Tilt = FQuat::Slerp(FQuat::Identity, FQuat::FindBetweenNormals(Up, Normal), Entry.AlignToNormal);
				const FQuat Yaw(Up, Entry.bRandomYaw ? FMath::DegreesToRadians(YawAlpha * 360.f) : 0.f);
				const FVector Location = FVector(P, Ground.WaterZ + Height) - Tilt.RotateVector(Up) * Entry.SinkDepth;

				OutPerEntry[EntryIndex].Add(FTransform(Tilt * Yaw, Location, FVector(Scale)));
			}
		}
	}
}
