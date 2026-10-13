#include "TerrainHeightField.h"

#include "Math/RandomStream.h"

namespace
{
	/** 1 while Dist <= Inner, smoothly down to 0 at Inner + Falloff. */
	float FalloffWeight(float Dist, float Inner, float Falloff)
	{
		if (Dist <= Inner)
		{
			return 1.f;
		}
		if (Falloff <= 0.f)
		{
			return 0.f;
		}
		return 1.f - FMath::SmoothStep(0.f, 1.f, (Dist - Inner) / Falloff);
	}

	/** Rescales Profile so its min..max over the corridor's X range spans [Low, High]. */
	void RemapProfile(const FTerrainGrid& Grid, float HalfX, float StartX, float EndX, float Low, float High, TArray<float>& Profile)
	{
		float BaseMin = TNumericLimits<float>::Max();
		float BaseMax = TNumericLimits<float>::Lowest();
		for (int32 X = 0; X < Grid.NumX; ++X)
		{
			const float PX = -HalfX + X * Grid.StepX;
			if (PX >= StartX && PX <= EndX)
			{
				BaseMin = FMath::Min(BaseMin, Profile[X]);
				BaseMax = FMath::Max(BaseMax, Profile[X]);
			}
		}
		const float BaseRange = BaseMax - BaseMin;
		for (float& H : Profile)
		{
			// Columns outside StartX..EndX (falloff only) extrapolate and are then clamped like the rest.
			H = BaseRange > UE_KINDA_SMALL_NUMBER ? Low + (H - BaseMin) / BaseRange * (High - Low) : (Low + High) * 0.5f;
		}
	}

	/**
	 * Scales Profile toward Low until the given share of its steps over StartX..EndX is at most MaxStep (never up).
	 * Uniform vertical scaling keeps the profile's shape: hills stay rounded and of different heights.
	 */
	void FitProfileSlope(const FTerrainGrid& Grid, float HalfX, float StartX, float EndX, float Low, float MaxStep, float Share, TArray<float>& Profile)
	{
		TArray<float> Steps;
		for (int32 X = 1; X < Grid.NumX; ++X)
		{
			const float PX = -HalfX + X * Grid.StepX;
			if (PX - Grid.StepX >= StartX && PX <= EndX)
			{
				Steps.Add(FMath::Abs(Profile[X] - Profile[X - 1]));
			}
		}
		if (Steps.IsEmpty())
		{
			return;
		}
		Steps.Sort();
		const float Step = Steps[FMath::Clamp(FMath::FloorToInt(Steps.Num() * Share), 0, Steps.Num() - 1)];
		if (Step <= MaxStep)
		{
			return;
		}
		const float Scale = MaxStep / Step;
		for (float& H : Profile)
		{
			H = Low + (H - Low) * Scale;
		}
	}

	/**
	 * Lowers Profile so neighbouring columns never differ by more than MaxStep. Lowering only (a forward and a backward
	 * pass) keeps every value inside its range and gives min_j(P[j] + d * |i - j|), which is slope-limited.
	 */
	void LimitProfileSlope(float MaxStep, TArray<float>& Profile)
	{
		for (int32 X = 1; X < Profile.Num(); ++X)
		{
			Profile[X] = FMath::Min(Profile[X], Profile[X - 1] + MaxStep);
		}
		for (int32 X = Profile.Num() - 2; X >= 0; --X)
		{
			Profile[X] = FMath::Min(Profile[X], Profile[X + 1] + MaxStep);
		}
	}

	/**
	 * Corridor profile per grid column: base height on the centre line (with the corridor's own noise frequency, if
	 * set), clamped (or with bRemapToRange, rescaled from its own min..max over StartX..EndX) to the corridor's range,
	 * then slope-limited. With RemapPasses > 1, rescale and limit again to win back the span the limit took. With
	 * SlopeFitShare, each rescale is followed by a uniform lowering that keeps the hills' shape (FitProfileSlope).
	 */
	void BuildCorridorProfile(const FTerrainHeightParams& Params, const FVector2D& NoiseOffset, const FTerrainGrid& Grid,
		const FTerrainCorridor& Corridor, TArray<float>& OutProfile)
	{
		const float HalfX = Params.SizeX * 0.5f;
		const bool bApplyMask = Params.Type == ETerrainType::Island && !Corridor.bOverrideShoreMask;
		const float Low = Params.WaterLevel + FMath::Min(Corridor.MinHeight, Corridor.MaxHeight);
		const float High = Params.WaterLevel + FMath::Max(Corridor.MinHeight, Corridor.MaxHeight);
		const float StartX = FMath::Min(Corridor.StartX, Corridor.EndX);
		const float EndX = FMath::Max(Corridor.StartX, Corridor.EndX);

		FTerrainHeightParams ProfileParams = Params;
		// The lane keeps its rolling profile (and its tested rules); ridges only shape the ground beside it.
		ProfileParams.RidgeWeight = 0.f;
		if (Corridor.ProfileNoiseScale > 0.f)
		{
			ProfileParams.NoiseScale = Corridor.ProfileNoiseScale;
		}
		if (Corridor.ProfileOctaves > 0)
		{
			ProfileParams.Octaves = Corridor.ProfileOctaves;
		}

		OutProfile.SetNumUninitialized(Grid.NumX);
		for (int32 X = 0; X < Grid.NumX; ++X)
		{
			OutProfile[X] = FTerrainHeightField::SampleBase(ProfileParams, NoiseOffset, -HalfX + X * Grid.StepX, Corridor.CenterY, bApplyMask);
		}

		const float MaxStep = FMath::Tan(FMath::DegreesToRadians(FMath::Clamp(Corridor.MaxSlopeDeg, 0.f, 89.f))) * Grid.StepX;
		const int32 Passes = Corridor.bRemapToRange ? FMath::Clamp(Corridor.RemapPasses, 1, 8) : 1;
		for (int32 Pass = 0; Pass < Passes; ++Pass)
		{
			if (Corridor.bRemapToRange)
			{
				RemapProfile(Grid, HalfX, StartX, EndX, Low, High, OutProfile);
				if (Corridor.SlopeFitShare > 0.f)
				{
					FitProfileSlope(Grid, HalfX, StartX, EndX, Low, MaxStep, Corridor.SlopeFitShare, OutProfile);
				}
			}
			for (float& H : OutProfile)
			{
				H = FMath::Clamp(H, Low, High);
			}
			LimitProfileSlope(MaxStep, OutProfile);
		}
	}

	/**
	 * FollowProfile banks, one grid column across Y: walking outward from each strip edge, each row may differ from the
	 * previous one by at most the cross slope, which grows from BankEdgeSlopeDeg at the edge to BankMaxSlopeDeg at
	 * BankRamp. Keeps the edge free of creases while the relief further out stays rough.
	 */
	void LimitBankSlope(const FTerrainCorridor& Corridor, const FTerrainGrid& Grid, float HalfY, TArray<float>& Column)
	{
		const float EdgeDeg = FMath::Clamp(Corridor.BankEdgeSlopeDeg, 0.f, 89.f);
		const float MaxDeg = FMath::Clamp(FMath::Max(Corridor.BankMaxSlopeDeg, EdgeDeg), 0.f, 89.f);
		auto MaxStepAt = [&](float Past)
		{
			const float T = Corridor.BankRamp > 0.f ? FMath::Clamp(Past / Corridor.BankRamp, 0.f, 1.f) : 1.f;
			return FMath::Tan(FMath::DegreesToRadians(FMath::Lerp(EdgeDeg, MaxDeg, T))) * Grid.StepY;
		};
		auto PastAt = [&](int32 Y) { return FMath::Abs(-HalfY + Y * Grid.StepY - Corridor.CenterY) - Corridor.HalfWidth; };
		auto Limit = [&](int32 Y, int32 Prev)
		{
			// The step's inner end sets its slope, so the first row past the edge gets exactly BankEdgeSlopeDeg.
			const float Step = MaxStepAt(PastAt(Prev));
			Column[Y] = FMath::Clamp(Column[Y], Column[Prev] - Step, Column[Prev] + Step);
		};
		for (int32 Y = 1; Y < Grid.NumY; ++Y)
		{
			if (-HalfY + Y * Grid.StepY > Corridor.CenterY && PastAt(Y) > 0.f)
			{
				Limit(Y, Y - 1);
			}
		}
		for (int32 Y = Grid.NumY - 2; Y >= 0; --Y)
		{
			if (-HalfY + Y * Grid.StepY < Corridor.CenterY && PastAt(Y) > 0.f)
			{
				Limit(Y, Y + 1);
			}
		}
	}

	/** Position across the shore beyond the corridor's land edges: 0 on land, 1 at the seabed (bDefineLandWidth). */
	float ShoreT(const FTerrainHeightParams& Params, const FVector2D& NoiseOffset, const FTerrainCorridor& Corridor, float PX, float PY, float Offset)
	{
		const float Land = Offset >= 0.f ? Corridor.LandPlusY : Corridor.LandMinusY;
		const float Shore = FMath::Max(Offset >= 0.f ? Corridor.ShorePlusY : Corridor.ShoreMinusY, 1.f);
		const float Past = FMath::Abs(Offset) - Land;
		if (Past <= 0.f)
		{
			return 0.f;
		}
		// Coastline noise only moves the drop within the shore, never into the land.
		const float Coast = FMath::PerlinNoise2D(FVector2D(PX, PY) * (Params.NoiseScale * 3.f) - NoiseOffset) * 0.5f + 0.5f;
		return FMath::Clamp(Past / Shore * (1.f + Params.CoastlineNoise * Coast), 0.f, 1.f);
	}

	/** Shore height at ShoreT T, from the land height Land: a smooth drop to the seabed, or descent + beach shelf + drop. */
	float ShoreHeight(const FTerrainHeightParams& Params, const FTerrainCorridor& Corridor, float Offset, float T, float Land)
	{
		const float Seabed = Params.WaterLevel - Params.SeaDepth;
		const float Shore = FMath::Max(Offset >= 0.f ? Corridor.ShorePlusY : Corridor.ShoreMinusY, 1.f);
		const float Beach = FMath::Clamp((Offset >= 0.f ? Corridor.BeachWidthPlusY : Corridor.BeachWidthMinusY) / Shore, 0.f, 0.9f);
		if (Beach <= 0.f)
		{
			return FMath::Lerp(Land, Seabed, FMath::SmoothStep(0.f, 1.f, T));
		}
		const float BeachZ = Params.WaterLevel + Corridor.BeachHeight;
		const float DescentEnd = 1.f - Beach;
		if (T < DescentEnd)
		{
			return FMath::Lerp(Land, BeachZ, FMath::SmoothStep(0.f, 1.f, T / DescentEnd));
		}
		// The shelf: 70 % a gentle slope down to the waterline, then the drop to the seabed.
		constexpr float ShelfShare = 0.7f;
		const float U = (T - DescentEnd) / Beach;
		return U < ShelfShare
			? FMath::Lerp(BeachZ, Params.WaterLevel, U / ShelfShare)
			: FMath::Lerp(Params.WaterLevel, Seabed, FMath::SmoothStep(0.f, 1.f, (U - ShelfShare) / (1.f - ShelfShare)));
	}

	/** One-sided height cap: at or below Profile + CapClearance on the capped side, blended in by Weight. */
	void ApplySideCap(const FTerrainCorridor& Corridor, float ProfileH, float Offset, float CapDir, float CapLength, float Weight, float& H)
	{
		if (!Corridor.bUseSideCap)
		{
			return;
		}
		const float Along = (Offset - Corridor.CapFromY) * CapDir;
		if (Along > 0.f)
		{
			const float CapWeight = Weight * FalloffWeight(Along, CapLength, Corridor.CapFalloff);
			H = FMath::Lerp(H, FMath::Min(H, ProfileH + Corridor.CapClearance), CapWeight);
		}
	}

	/** bDefineLandWidth: dry land never below the corridor's lowest height; past the edges, down to the seabed. */
	void ApplyLandWidth(const FTerrainHeightParams& Params, const FVector2D& NoiseOffset, const FTerrainCorridor& Corridor, float Low,
		float PX, float PY, float Offset, float Weight, float& H)
	{
		const float Shaped = ShoreHeight(Params, Corridor, Offset, ShoreT(Params, NoiseOffset, Corridor, PX, PY, Offset), FMath::Max(H, Low));
		H = FMath::Lerp(H, Shaped, Weight);
	}

	void ApplyCorridor(const FTerrainHeightParams& Params, const FVector2D& NoiseOffset, const FTerrainGrid& Grid,
		const FTerrainCorridor& Corridor, TArray<float>& Heights)
	{
		TArray<float> Profile;
		BuildCorridorProfile(Params, NoiseOffset, Grid, Corridor, Profile);

		const float HalfX = Params.SizeX * 0.5f;
		const float HalfY = Params.SizeY * 0.5f;
		const float StartX = FMath::Min(Corridor.StartX, Corridor.EndX);
		const float EndX = FMath::Max(Corridor.StartX, Corridor.EndX);
		const float Low = Params.WaterLevel + FMath::Min(Corridor.MinHeight, Corridor.MaxHeight);

		const float CapDir = Corridor.CapToY >= Corridor.CapFromY ? 1.f : -1.f;
		const float CapLength = FMath::Abs(Corridor.CapToY - Corridor.CapFromY);
		const bool bFollow = Corridor.BankMode == ETerrainCorridorBankMode::FollowProfile;
		const bool bLandWidth = Corridor.bDefineLandWidth && Params.Type == ETerrainType::Island;
		TArray<float> Column;

		for (int32 X = 0; X < Grid.NumX; ++X)
		{
			const float PX = -HalfX + X * Grid.StepX;
			const float OutsideX = FMath::Max3(StartX - PX, PX - EndX, 0.f);
			const float WeightX = FalloffWeight(OutsideX, 0.f, Corridor.Falloff);
			if (WeightX <= 0.f)
			{
				continue;
			}

			if (bFollow)
			{
				// FollowProfile: the strip's profile plus the terrain's relief (against its own centre line, unmasked),
				// eased in from the strip edge.
				const float RawCentre = FTerrainHeightField::SampleBase(Params, NoiseOffset, PX, Corridor.CenterY, false);
				Column.SetNumUninitialized(Grid.NumY);
				for (int32 Y = 0; Y < Grid.NumY; ++Y)
				{
					const float PY = -HalfY + Y * Grid.StepY;
					const float Offset = PY - Corridor.CenterY;
					const float Past = FMath::Abs(Offset) - Corridor.HalfWidth;
					Column[Y] = Profile[X];
					if (Past > 0.f)
					{
						const float Ramp = Corridor.BankRamp > 0.f ? FMath::SmoothStep(0.f, 1.f, Past / Corridor.BankRamp) : 1.f;
						const float Scale = Offset >= 0.f ? Corridor.BankScalePlus : Corridor.BankScaleMinus;
						Column[Y] += Ramp * Scale * (FTerrainHeightField::SampleBase(Params, NoiseOffset, PX, PY, false) - RawCentre);
					}
				}
				if (Corridor.BankEdgeSlopeDeg > 0.f)
				{
					LimitBankSlope(Corridor, Grid, HalfY, Column);
				}
			}

			for (int32 Y = 0; Y < Grid.NumY; ++Y)
			{
				const float PY = -HalfY + Y * Grid.StepY;
				const float Offset = PY - Corridor.CenterY;
				float& H = Heights[Y * Grid.NumX + X];

				if (!bFollow)
				{
					// BlendToRaw: unchanged from before banks existed, so existing presets keep their heights.
					const float Weight = WeightX * FalloffWeight(FMath::Abs(Offset), Corridor.HalfWidth, Corridor.Falloff);
					H = FMath::Lerp(H, Profile[X], Weight);
					ApplySideCap(Corridor, Profile[X], Offset, CapDir, CapLength, WeightX, H);
					if (bLandWidth)
					{
						ApplyLandWidth(Params, NoiseOffset, Corridor, Low, PX, PY, Offset, WeightX, H);
					}
					continue;
				}

				float Shaped = Column[Y];
				ApplySideCap(Corridor, Profile[X], Offset, CapDir, CapLength, 1.f, Shaped);
				if (bLandWidth)
				{
					ApplyLandWidth(Params, NoiseOffset, Corridor, Low, PX, PY, Offset, 1.f, Shaped);
				}
				H = FMath::Lerp(H, Shaped, WeightX);
			}
		}
	}
}

FTerrainGrid FTerrainGrid::Make(float SizeX, float SizeY, float CellSizeX, float CellSizeY)
{
	FTerrainGrid Grid;
	Grid.NumX = FMath::Max(2, FMath::CeilToInt(SizeX / FMath::Max(CellSizeX, 1.f)) + 1);
	Grid.NumY = FMath::Max(2, FMath::CeilToInt(SizeY / FMath::Max(CellSizeY, 1.f)) + 1);
	Grid.StepX = SizeX / (Grid.NumX - 1);
	Grid.StepY = SizeY / (Grid.NumY - 1);
	return Grid;
}

FVector2D FTerrainHeightField::MakeNoiseOffset(int32 Seed)
{
	// The original code drew both inside one FVector2D(...) call. MSVC evaluates those arguments right to left,
	// so the first draw went to Y. Spelled out here so existing seeds keep their shape and every compiler agrees.
	FRandomStream Stream(Seed);
	// Kept in double: FRandRange returns double, and rounding it to float shifts every height slightly.
	const double OffsetY = Stream.FRandRange(-1000.f, 1000.f);
	const double OffsetX = Stream.FRandRange(-1000.f, 1000.f);
	return FVector2D(OffsetX, OffsetY);
}

float FTerrainHeightField::SampleBase(const FTerrainHeightParams& Params, const FVector2D& NoiseOffset, float X, float Y, bool bApplyMask)
{
	// Fractal Brownian motion: sum of Perlin octaves -> smooth rolling hills.
	float Amplitude = 1.f;
	float Frequency = Params.NoiseScale;
	float Sum = 0.f;
	float Norm = 0.f;
	// Ridged fBm from the same octaves: (1 - |noise|)^2, each octave weighted by the one before, so detail gathers on ridges.
	float RidgeSum = 0.f;
	float RidgePrev = 1.f;
	const bool bRidges = Params.RidgeWeight > 0.f;
	for (int32 Octave = 0; Octave < Params.Octaves; ++Octave)
	{
		const float Noise = FMath::PerlinNoise2D(FVector2D(X, Y) * Frequency + NoiseOffset);
		Sum += Amplitude * Noise;
		if (bRidges)
		{
			const float Ridge = FMath::Square(1.f - FMath::Abs(Noise)) * RidgePrev;
			RidgeSum += Amplitude * Ridge;
			RidgePrev = FMath::Clamp(Ridge * 2.f, 0.f, 1.f);
		}
		Norm += Amplitude;
		Amplitude *= Params.Persistence;
		Frequency *= Params.Lacunarity;
	}
	// Perlin output rarely reaches +-1, so stretch it a bit before mapping to [0,1].
	float H = FMath::Clamp((Sum / Norm) * 0.7f + 0.5f, 0.f, 1.f);
	if (bRidges)
	{
		// Ridges and peaks on the high ground only; the low ground stays rolling.
		const float Blend = FMath::Clamp(Params.RidgeWeight, 0.f, 1.f) * FMath::SmoothStep(Params.RidgeStart, 1.f, H);
		H = FMath::Lerp(H, FMath::Clamp(RidgeSum / Norm, 0.f, 1.f), Blend);
	}
	H = FMath::Pow(H, Params.Flatness);

	// Inland ground never goes below WaterLevel + MinLandHeight (negative = shallow ponds / dry valleys).
	const float LandZ = Params.WaterLevel + Params.MinLandHeight + H * (Params.MaxHeight - Params.MinLandHeight);
	if (!bApplyMask)
	{
		return LandZ;
	}

	// Island mask: 0 at the border (under water), 1 in the interior.
	const float HalfX = Params.SizeX * 0.5f;
	const float HalfY = Params.SizeY * 0.5f;
	const float EdgeX = (HalfX - FMath::Abs(X)) / (HalfX * Params.ShoreWidth);
	const float EdgeY = (HalfY - FMath::Abs(Y)) / (HalfY * Params.ShoreWidth);
	float Edge = FMath::Min(EdgeX, EdgeY);

	// Only ever pushes the shore inward, so the mesh border always stays under water.
	const float Coast = FMath::PerlinNoise2D(FVector2D(X, Y) * (Params.NoiseScale * 3.f) - NoiseOffset);
	Edge -= Params.CoastlineNoise * (Coast * 0.5f + 0.5f);

	const float Mask = FMath::SmoothStep(0.f, 1.f, Edge);
	return FMath::Lerp(Params.WaterLevel - Params.SeaDepth, LandZ, Mask);
}

void FTerrainHeightField::Build(const FTerrainHeightParams& Params, const FTerrainGrid& Grid, TArray<float>& OutHeights)
{
	OutHeights.Reset();
	if (Params.Type == ETerrainType::WaterOnly || Grid.NumX < 2 || Grid.NumY < 2)
	{
		return;
	}

	const FVector2D NoiseOffset = MakeNoiseOffset(Params.Seed);
	const bool bApplyMask = Params.Type == ETerrainType::Island;
	const float HalfX = Params.SizeX * 0.5f;
	const float HalfY = Params.SizeY * 0.5f;

	OutHeights.SetNumUninitialized(Grid.NumX * Grid.NumY);
	for (int32 Y = 0; Y < Grid.NumY; ++Y)
	{
		for (int32 X = 0; X < Grid.NumX; ++X)
		{
			OutHeights[Y * Grid.NumX + X] = SampleBase(Params, NoiseOffset, -HalfX + X * Grid.StepX, -HalfY + Y * Grid.StepY, bApplyMask);
		}
	}

	for (const FTerrainCorridor& Corridor : Params.Corridors)
	{
		ApplyCorridor(Params, NoiseOffset, Grid, Corridor, OutHeights);
	}
}

void FTerrainHeightField::ComputeOcclusion(const TArray<float>& Heights, const FTerrainGrid& Grid, float SizeX, float SizeY, float Radius, float Depth,
	TArray<float>& OutOcclusion)
{
	OutOcclusion.Init(0.f, Heights.Num());
	if (Radius <= 0.f || Depth <= 0.f || Grid.NumX < 2 || Grid.NumY < 2 || Heights.Num() != Grid.NumX * Grid.NumY)
	{
		return;
	}

	constexpr int32 NumDirections = 8;
	FVector2D Offsets[NumDirections * 2];
	for (int32 Dir = 0; Dir < NumDirections; ++Dir)
	{
		const float Angle = 2.f * PI * Dir / NumDirections;
		const FVector2D Unit(FMath::Cos(Angle), FMath::Sin(Angle));
		Offsets[Dir * 2] = Unit * Radius;
		Offsets[Dir * 2 + 1] = Unit * (Radius * 0.5f);
	}

	const float HalfX = SizeX * 0.5f;
	const float HalfY = SizeY * 0.5f;
	for (int32 Y = 0; Y < Grid.NumY; ++Y)
	{
		for (int32 X = 0; X < Grid.NumX; ++X)
		{
			const FVector2D P(-HalfX + X * Grid.StepX, -HalfY + Y * Grid.StepY);
			float Sum = 0.f;
			// Samples past the edge clamp to it (SampleGrid), so borders read as open, never as enclosed.
			for (const FVector2D& Offset : Offsets)
			{
				Sum += SampleGrid(Heights, Grid, SizeX, SizeY, P.X + Offset.X, P.Y + Offset.Y);
			}
			const int32 Index = Y * Grid.NumX + X;
			OutOcclusion[Index] = FMath::Clamp((Sum / UE_ARRAY_COUNT(Offsets) - Heights[Index]) / Depth, 0.f, 1.f);
		}
	}
}

float FTerrainHeightField::PathWeight(const FTerrainCorridor& Corridor, float HalfWidth, float LocalX, float LocalY)
{
	if (HalfWidth <= 0.f)
	{
		return 0.f;
	}
	const float StartX = FMath::Min(Corridor.StartX, Corridor.EndX);
	const float EndX = FMath::Max(Corridor.StartX, Corridor.EndX);
	const float WeightX = FalloffWeight(FMath::Max3(StartX - LocalX, LocalX - EndX, 0.f), 0.f, Corridor.Falloff);
	return WeightX * FMath::Clamp(1.f - FMath::Abs(LocalY - Corridor.CenterY) / HalfWidth, 0.f, 1.f);
}

void FTerrainHeightField::AddContactDarkening(const FTerrainGrid& Grid, float SizeX, float SizeY, const FVector2D& Center, float Radius, float Strength,
	TArray<float>& InOutDarkness)
{
	if (Radius <= 0.f || Strength <= 0.f || Grid.NumX < 2 || Grid.NumY < 2 || InOutDarkness.Num() != Grid.NumX * Grid.NumY)
	{
		return;
	}

	const float HalfX = SizeX * 0.5f;
	const float HalfY = SizeY * 0.5f;
	const int32 X0 = FMath::Clamp(FMath::FloorToInt((Center.X - Radius + HalfX) / Grid.StepX), 0, Grid.NumX - 1);
	const int32 X1 = FMath::Clamp(FMath::CeilToInt((Center.X + Radius + HalfX) / Grid.StepX), 0, Grid.NumX - 1);
	const int32 Y0 = FMath::Clamp(FMath::FloorToInt((Center.Y - Radius + HalfY) / Grid.StepY), 0, Grid.NumY - 1);
	const int32 Y1 = FMath::Clamp(FMath::CeilToInt((Center.Y + Radius + HalfY) / Grid.StepY), 0, Grid.NumY - 1);
	for (int32 Y = Y0; Y <= Y1; ++Y)
	{
		for (int32 X = X0; X <= X1; ++X)
		{
			const float Dist = FVector2D::Distance(Center, FVector2D(-HalfX + X * Grid.StepX, -HalfY + Y * Grid.StepY));
			if (Dist < Radius)
			{
				float& Darkness = InOutDarkness[Y * Grid.NumX + X];
				Darkness = FMath::Max(Darkness, Strength * FMath::Square(1.f - Dist / Radius));
			}
		}
	}
}

float FTerrainHeightField::SampleGrid(const TArray<float>& Heights, const FTerrainGrid& Grid, float SizeX, float SizeY, float LocalX, float LocalY)
{
	if (Grid.NumX < 2 || Grid.NumY < 2 || Heights.Num() != Grid.NumX * Grid.NumY)
	{
		return 0.f;
	}

	const float FX = FMath::Clamp((LocalX + SizeX * 0.5f) / Grid.StepX, 0.f, float(Grid.NumX - 1));
	const float FY = FMath::Clamp((LocalY + SizeY * 0.5f) / Grid.StepY, 0.f, float(Grid.NumY - 1));
	const int32 X0 = FMath::Min(FMath::FloorToInt(FX), Grid.NumX - 2);
	const int32 Y0 = FMath::Min(FMath::FloorToInt(FY), Grid.NumY - 2);
	const float TX = FX - X0;
	const float TY = FY - Y0;

	const float H00 = Heights[Y0 * Grid.NumX + X0];
	const float H10 = Heights[Y0 * Grid.NumX + X0 + 1];
	const float H01 = Heights[(Y0 + 1) * Grid.NumX + X0];
	const float H11 = Heights[(Y0 + 1) * Grid.NumX + X0 + 1];

	// Interpolate on the same triangle the mesh uses, so actors sit exactly on the surface.
	return (TX + TY <= 1.f)
		? H00 + TX * (H10 - H00) + TY * (H01 - H00)
		: H11 + (1.f - TX) * (H01 - H11) + (1.f - TY) * (H10 - H11);
}
