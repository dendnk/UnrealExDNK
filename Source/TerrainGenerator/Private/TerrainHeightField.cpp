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

	/**
	 * Corridor profile per grid column: base height on the centre line, clamped (or with bRemapToRange, rescaled from
	 * its own min..max over StartX..EndX) to the corridor's range, then
	 * lowered so neighbouring columns never differ by more than tan(MaxSlope) * StepX. Lowering only (a forward and
	 * a backward pass) keeps every value inside [Min, Max] and gives min_j(P[j] + d * |i - j|), which is slope-limited.
	 */
	void BuildCorridorProfile(const FTerrainHeightParams& Params, const FVector2D& NoiseOffset, const FTerrainGrid& Grid,
		const FTerrainCorridor& Corridor, TArray<float>& OutProfile)
	{
		const float HalfX = Params.SizeX * 0.5f;
		const bool bApplyMask = Params.Type == ETerrainType::Island && !Corridor.bOverrideShoreMask;
		const float Low = Params.WaterLevel + FMath::Min(Corridor.MinHeight, Corridor.MaxHeight);
		const float High = Params.WaterLevel + FMath::Max(Corridor.MinHeight, Corridor.MaxHeight);

		OutProfile.SetNumUninitialized(Grid.NumX);
		for (int32 X = 0; X < Grid.NumX; ++X)
		{
			OutProfile[X] = FTerrainHeightField::SampleBase(Params, NoiseOffset, -HalfX + X * Grid.StepX, Corridor.CenterY, bApplyMask);
		}

		if (Corridor.bRemapToRange)
		{
			const float StartX = FMath::Min(Corridor.StartX, Corridor.EndX);
			const float EndX = FMath::Max(Corridor.StartX, Corridor.EndX);
			float BaseMin = TNumericLimits<float>::Max();
			float BaseMax = TNumericLimits<float>::Lowest();
			for (int32 X = 0; X < Grid.NumX; ++X)
			{
				const float PX = -HalfX + X * Grid.StepX;
				if (PX >= StartX && PX <= EndX)
				{
					BaseMin = FMath::Min(BaseMin, OutProfile[X]);
					BaseMax = FMath::Max(BaseMax, OutProfile[X]);
				}
			}
			const float BaseRange = BaseMax - BaseMin;
			for (float& H : OutProfile)
			{
				// Columns outside StartX..EndX (falloff only) extrapolate and are then clamped like the rest.
				H = BaseRange > UE_KINDA_SMALL_NUMBER ? Low + (H - BaseMin) / BaseRange * (High - Low) : (Low + High) * 0.5f;
			}
		}
		for (float& H : OutProfile)
		{
			H = FMath::Clamp(H, Low, High);
		}

		const float MaxStep = FMath::Tan(FMath::DegreesToRadians(FMath::Clamp(Corridor.MaxSlopeDeg, 0.f, 89.f))) * Grid.StepX;
		for (int32 X = 1; X < Grid.NumX; ++X)
		{
			OutProfile[X] = FMath::Min(OutProfile[X], OutProfile[X - 1] + MaxStep);
		}
		for (int32 X = Grid.NumX - 2; X >= 0; --X)
		{
			OutProfile[X] = FMath::Min(OutProfile[X], OutProfile[X + 1] + MaxStep);
		}
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

		const float CapDir = Corridor.CapToY >= Corridor.CapFromY ? 1.f : -1.f;
		const float CapLength = FMath::Abs(Corridor.CapToY - Corridor.CapFromY);

		for (int32 X = 0; X < Grid.NumX; ++X)
		{
			const float PX = -HalfX + X * Grid.StepX;
			const float OutsideX = FMath::Max3(StartX - PX, PX - EndX, 0.f);
			const float WeightX = FalloffWeight(OutsideX, 0.f, Corridor.Falloff);
			if (WeightX <= 0.f)
			{
				continue;
			}

			for (int32 Y = 0; Y < Grid.NumY; ++Y)
			{
				const float Offset = -HalfY + Y * Grid.StepY - Corridor.CenterY;
				float& H = Heights[Y * Grid.NumX + X];

				const float Weight = WeightX * FalloffWeight(FMath::Abs(Offset), Corridor.HalfWidth, Corridor.Falloff);
				H = FMath::Lerp(H, Profile[X], Weight);

				if (Corridor.bUseSideCap)
				{
					const float Along = (Offset - Corridor.CapFromY) * CapDir;
					if (Along > 0.f)
					{
						const float CapWeight = WeightX * FalloffWeight(Along, CapLength, Corridor.CapFalloff);
						H = FMath::Lerp(H, FMath::Min(H, Profile[X] + Corridor.CapClearance), CapWeight);
					}
				}
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
	for (int32 Octave = 0; Octave < Params.Octaves; ++Octave)
	{
		Sum += Amplitude * FMath::PerlinNoise2D(FVector2D(X, Y) * Frequency + NoiseOffset);
		Norm += Amplitude;
		Amplitude *= Params.Persistence;
		Frequency *= Params.Lacunarity;
	}
	// Perlin output rarely reaches +-1, so stretch it a bit before mapping to [0,1].
	float H = FMath::Clamp((Sum / Norm) * 0.7f + 0.5f, 0.f, 1.f);
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
