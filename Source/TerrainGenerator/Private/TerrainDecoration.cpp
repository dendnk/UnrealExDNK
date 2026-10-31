#include "TerrainDecoration.h"

#include "TerrainHeightField.h"

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

	/**
	 * How far a base at CentreHeight must drop (<= 0) so the plane through it with normal PlaneUp is at or below the
	 * ground everywhere on a disc of Radius: two rings, the rim and half way.
	 */
	float ContactDrop(const FTerrainDecorationGround& Ground, const FVector2D& Centre, float CentreHeight, float Radius, const FVector& PlaneUp)
	{
		constexpr int32 Rings = 2;
		constexpr int32 PointsPerRing = 12;
		const float InvZ = 1.f / FMath::Max(PlaneUp.Z, 0.1f);
		float Drop = 0.f;
		for (int32 Ring = 1; Ring <= Rings; ++Ring)
		{
			const float R = Radius * Ring / Rings;
			for (int32 Point = 0; Point < PointsPerRing; ++Point)
			{
				const float Angle = 2.f * PI * (Point + 0.5f * (Ring & 1)) / PointsPerRing;
				const FVector2D Offset = R * FVector2D(FMath::Cos(Angle), FMath::Sin(Angle));
				const float PlaneHeight = CentreHeight - (PlaneUp.X * Offset.X + PlaneUp.Y * Offset.Y) * InvZ;
				Drop = FMath::Min(Drop, Ground.HeightAboveWater(Centre + Offset) - PlaneHeight);
			}
		}
		return Drop;
	}

	/** Lowest and highest ground under a footprint circle: its centre plus rings at a quarter, half, three quarters and the rim. */
	void FootprintHeights(const FTerrainDecorationGround& Ground, const FVector2D& Centre, float Radius, float& OutLow, float& OutHigh)
	{
		constexpr int32 Rings = 4;
		constexpr int32 PointsPerRing = 16;
		OutLow = OutHigh = Ground.HeightAboveWater(Centre);
		for (int32 Ring = 1; Ring <= Rings; ++Ring)
		{
			const float R = Radius * Ring / Rings;
			for (int32 Point = 0; Point < PointsPerRing; ++Point)
			{
				const float Angle = 2.f * PI * (Point + 0.5f * (Ring & 1)) / PointsPerRing;
				const float H = Ground.HeightAboveWater(Centre + R * FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)));
				OutLow = FMath::Min(OutLow, H);
				OutHigh = FMath::Max(OutHigh, H);
			}
		}
	}
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
		const float Side = (Band.Side != 0 ? Band.Side : Set.BandSideSign) >= 0 ? 1.f : -1.f;
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
				const float Radius = Info.RadiusXY * Scale;
				// The sampled distance is the bounds' centre, or with bPlaceByInnerEdge their edge toward the corridor.
				const float CentreDistance = Band.bPlaceByInnerEdge ? Distance + Radius : Distance;
				if (CentreDistance - Radius < Set.MinLaneDistance)
				{
					continue;
				}
				if ((Band.bContainInnerEdge && CentreDistance - Radius < MinDistance) || (Band.bContainOuterEdge && CentreDistance + Radius > MaxDistance))
				{
					continue;
				}
				if (Band.MaxMeshHeight > 0.f && Info.Height * Scale > Band.MaxMeshHeight)
				{
					continue;
				}
				// Nothing stands on a cliff face or in a rough cliff's zone, which run across the whole terrain
				// (specs/<status>-terrain-cliff-drops; the zone's own rocks come from ScatterCliffRocks). The clearance covers
				// the bounds centre moving a little when the mesh tilts to the ground.
				if (Lane.Cliffs.ContainsByPredicate([X, Radius, &Lane](const FTerrainCliff& Cliff)
					{
						constexpr float CliffClearance = 50.f;
						const FVector2D Zone = FTerrainHeightField::CliffZone(Lane, Cliff);
						return X - Radius - CliffClearance < Zone.Y && X + Radius + CliffClearance > Zone.X;
					}))
				{
					continue;
				}

				const FVector2D P(X, Lane.CenterY + Side * CentreDistance);
				// Grounded footprints are checked by their highest ground (some of it on land) and stand on their lowest.
				float Height = Ground.HeightAboveWater(P);
				float BaseHeight = Height;
				if (Entry.bGroundFootprint)
				{
					float Low = 0.f;
					FootprintHeights(Ground, P, Radius, Low, Height);
					BaseHeight = Ground.bHasWater ? FMath::Max(Low, 0.f) : Low;
				}
				const float MinHeight = Ground.bHasWater ? FMath::Max(0.f, Entry.HeightAboveWater.X) : Entry.HeightAboveWater.X;
				if (Height < MinHeight || Height > Entry.HeightAboveWater.Y)
				{
					continue;
				}

				const FVector Normal = Ground.Normal(P);
				if (!Entry.bGroundFootprint && Normal.Z < FMath::Cos(FMath::DegreesToRadians(Entry.MaxSlopeDeg)))
				{
					continue;
				}

				if (!Spacing.IsFree(P, Entry.MinSpacing, EntryIndex))
				{
					continue;
				}
				Spacing.Add(P, Entry.MinSpacing, EntryIndex);

				const FQuat Tilt = FQuat::Slerp(FQuat::Identity, FQuat::FindBetweenNormals(Up, Normal), Entry.AlignToNormal);
				// Stand on the contact disc, not just its centre: the downhill side of a wide rock or a trunk never floats.
				if (!Entry.bGroundFootprint && Entry.ContactRadiusFraction > 0.f)
				{
					BaseHeight += ContactDrop(Ground, P, BaseHeight, Radius * Entry.ContactRadiusFraction, Tilt.RotateVector(Up));
				}
				const FQuat Yaw(Up, Entry.bRandomYaw ? FMath::DegreesToRadians(YawAlpha * 360.f) : 0.f);
				// The mesh's bounds centre lands on the sample; the pivot goes wherever that puts it.
				const FVector CenterOffset = (Tilt * Yaw).RotateVector(FVector(Info.CenterXY * Scale, 0.f));
				const FVector Location = FVector(P, Ground.WaterZ + BaseHeight) - CenterOffset - Tilt.RotateVector(Up) * Entry.SinkDepth;

				OutPerEntry[EntryIndex].Add(FTransform(Tilt * Yaw, Location, FVector(Scale)));
			}
		}
	}
}

FTransform FTerrainDecorationScatter::CollisionTransform(const FTransform& Visual, const FVector& MeshCenter, float CollisionScale)
{
	// Same rotation; the bounds centre stays where it is while everything shrinks toward it.
	const FVector Scale = Visual.GetScale3D();
	const FVector Shift = Visual.GetRotation().RotateVector(Scale * MeshCenter * (1.f - CollisionScale));
	return FTransform(Visual.GetRotation(), Visual.GetLocation() + Shift, Scale * CollisionScale);
}

void FTerrainDecorationScatter::ScatterCliffRocks(const UTerrainDecorationSet& Set, TConstArrayView<FTerrainDecorationMeshInfo> MeshInfo,
	const FTerrainCorridor& Lane, const FTerrainDecorationGround& Ground, int32 Seed, TArray<FTerrainCliffRock>& OutRocks)
{
	OutRocks.Reset();
	const FTerrainCliffRockSettings& Rocks = Set.CliffRocks;
	if (Lane.Cliffs.IsEmpty() || MeshInfo.Num() != Set.Entries.Num() || !Ground.HeightAboveWater)
	{
		return;
	}

	TArray<int32> Candidates;
	float TotalWeight = 0.f;
	for (const int32 Index : Rocks.EntryIndices)
	{
		if (Set.Entries.IsValidIndex(Index) && Set.Entries[Index].Mesh != nullptr && Set.Entries[Index].Weight > 0.f
			&& MeshInfo[Index].RadiusXY > 0.f && !Candidates.Contains(Index))
		{
			Candidates.Add(Index);
			TotalWeight += Set.Entries[Index].Weight;
		}
	}
	const float ToeWeight = FMath::Max(Rocks.ToeWeight, 0.f);
	const float FaceWeight = FMath::Max(Rocks.FaceWeight, 0.f);
	const float KindTotal = ToeWeight + FaceWeight + FMath::Max(Rocks.LipWeight, 0.f);
	if (Candidates.IsEmpty() || KindTotal <= 0.f)
	{
		return;
	}
	const float SpacingMin = FMath::Max(FMath::Min(Rocks.Spacing.X, Rocks.Spacing.Y), 50.f);
	const float SpacingMax = FMath::Max(FMath::Max(Rocks.Spacing.X, Rocks.Spacing.Y), SpacingMin);
	const float YFrom = Lane.CenterY - Rocks.ReachMinusY;
	const float YTo = Lane.CenterY + Rocks.ReachPlusY;
	const FVector Up = FVector::UpVector;
	auto GroundZ = [&Ground](float X, float Y) { return Ground.WaterZ + Ground.HeightAboveWater(FVector2D(X, Y)); };
	// Lowest ground under a footprint: centre, rim and half way, 8 directions.
	auto LowestZ = [&GroundZ](float X, float Y, float Radius)
	{
		float Low = GroundZ(X, Y);
		for (int32 Ring = 1; Ring <= 2; ++Ring)
		{
			for (int32 Point = 0; Point < 8; ++Point)
			{
				const float Angle = UE_PI * 0.25f * Point;
				const float R = Radius * Ring * 0.5f;
				Low = FMath::Min(Low, GroundZ(X + R * FMath::Cos(Angle), Y + R * FMath::Sin(Angle)));
			}
		}
		return Low;
	};

	for (const FTerrainCliff& Cliff : Lane.Cliffs)
	{
		FRandomStream Stream(int32(HashCombine(GetTypeHash(Seed), HashCombine(GetTypeHash(Cliff.StartX), GetTypeHash(Cliff.Rise)))));
		// Dir points from the high side to the low side along X.
		const float Dir = Cliff.Rise <= 0.f ? 1.f : -1.f;
		const float TopX = Dir > 0.f ? Cliff.StartX : Cliff.EndX();
		const float BottomX = Dir > 0.f ? Cliff.EndX() : Cliff.StartX;
		const float Run = FMath::Max(Cliff.Run, 1.f);
		const float FaceAngle = FMath::Atan(FMath::Abs(Cliff.Rise) / Run);
		const FVector FaceNormal(Dir * FMath::Sin(FaceAngle), 0.f, FMath::Cos(FaceAngle));
		// The enemy keep-out on X (HeliAce MakeKeepOuts): the face plus the high and low margins.
		const float KeepFrom = Cliff.StartX - (Dir > 0.f ? Lane.CliffMarginHigh : Lane.CliffMarginLow);
		const float KeepTo = Cliff.EndX() + (Dir > 0.f ? Lane.CliffMarginLow : Lane.CliffMarginHigh);

		for (float Y = YFrom + float(Stream.FRandRange(0.0, SpacingMin)); Y <= YTo; Y += float(Stream.FRandRange(SpacingMin, SpacingMax)))
		{
			// Always the same draws per sample, so a rejected rock never shifts the rest of the cliff.
			const float KindPick = Stream.FRand() * KindTotal;
			const float EntryPick = Stream.FRand() * TotalWeight;
			const float SizeAlpha = Stream.FRand();
			const float PosAlpha = Stream.FRand();
			const float YawAlpha = Stream.FRand();
			const float TiltAlpha = Stream.FRand();
			const float TiltDirAlpha = Stream.FRand();
			const float JitterY = float(Stream.FRandRange(-0.25, 0.25)) * SpacingMin;

			const ETerrainCliffRockKind Kind = KindPick < ToeWeight ? ETerrainCliffRockKind::Toe
				: KindPick < ToeWeight + FaceWeight ? ETerrainCliffRockKind::Face : ETerrainCliffRockKind::Lip;
			int32 EntryIndex = Candidates.Last();
			float Accumulated = 0.f;
			for (const int32 Candidate : Candidates)
			{
				Accumulated += Set.Entries[Candidate].Weight;
				if (EntryPick < Accumulated)
				{
					EntryIndex = Candidate;
					break;
				}
			}
			const FTerrainDecorationMeshInfo& Info = MeshInfo[EntryIndex];
			const FVector2f SizeRange = Kind == ETerrainCliffRockKind::Toe ? Rocks.ToeSize
				: Kind == ETerrainCliffRockKind::Face ? Rocks.FaceSize : Rocks.LipSize;
			const float Width = FMath::Max(FMath::Lerp(SizeRange.X, SizeRange.Y, SizeAlpha), 10.f);
			const float Scale = Width / (2.f * Info.RadiusXY);
			const float Radius = Info.RadiusXY * Scale;
			const float HalfZ = FMath::Max(Info.ExtentZ * Scale, 1.f);
			// Bounds can tilt: the farthest a corner gets from the centre across X.
			const float Reach = FMath::Sqrt(FMath::Square(Radius) + FMath::Square(HalfZ));
			const float PY = Y + JitterY;

			FVector Center;
			switch (Kind)
			{
			case ETerrainCliffRockKind::Toe:
			{
				// Fallen boulders just past the bottom edge, leaning into the toe; a fifth of their height sunk.
				const float CX = BottomX + Dir * Radius * (0.3f + 0.8f * PosAlpha);
				Center = FVector(CX, PY, LowestZ(CX, PY, Radius) - 0.4f * HalfZ + HalfZ);
				break;
			}
			case ETerrainCliffRockKind::Face:
			{
				// Down the face, mostly sunk into it: the centre sits a little out along the face normal.
				const float CX = TopX + Dir * Run * (0.2f + 0.6f * PosAlpha);
				Center = FVector(CX, PY, GroundZ(CX, PY)) + FaceNormal * (0.2f * FMath::Min(Radius, HalfZ));
				break;
			}
			default:
			{
				// On the high ground right behind the top edge, its front over the broken lip; a third of its height sunk.
				const float CX = TopX - Dir * Radius * (0.1f + 0.4f * PosAlpha);
				Center = FVector(CX, PY, GroundZ(CX, PY) - 0.6f * HalfZ + HalfZ);
				break;
			}
			}

			if (Center.X - Reach < KeepFrom || Center.X + Reach > KeepTo)
			{
				continue;
			}
			// The ground right under the centre must cut the rock: a face rock pushed out along the nominal normal is kept
			// part sunk on a facet steeper than the mean face, and a toe rock whose footprint reaches far below its centre
			// (a ledge, the shore) would be buried on its high side, so it is dropped.
			const float CenterGround = GroundZ(Center.X, Center.Y);
			if (Kind == ETerrainCliffRockKind::Face)
			{
				Center.Z = FMath::Clamp(Center.Z, CenterGround - 0.6f * HalfZ, CenterGround + 0.6f * HalfZ);
			}
			else if (Center.Z + 0.6f * HalfZ < CenterGround || Center.Z - HalfZ > CenterGround)
			{
				continue;
			}
			if (Ground.bHasWater && Ground.HeightAboveWater(FVector2D(Center.X, PY)) < 30.f)
			{
				continue;
			}

			const float TiltDir = TiltDirAlpha * 2.f * UE_PI;
			const FQuat Tilt(FVector(FMath::Cos(TiltDir), FMath::Sin(TiltDir), 0.f), FMath::DegreesToRadians(TiltAlpha * Rocks.MaxTiltDeg));
			const FQuat Rotation = Tilt * FQuat(Up, YawAlpha * 2.f * UE_PI);
			const FVector MeshCenter(Info.CenterXY, Info.CenterZ);

			FTerrainCliffRock& Rock = OutRocks.AddDefaulted_GetRef();
			Rock.EntryIndex = EntryIndex;
			Rock.Kind = Kind;
			Rock.Radius = Radius;
			Rock.Center = Center;
			Rock.Transform = FTransform(Rotation, Center - Rotation.RotateVector(MeshCenter * Scale), FVector(Scale));
			Rock.bSolid = FMath::Abs(PY - Lane.CenterY) - Reach <= Rocks.SolidHalfWidth;
		}
	}
}
