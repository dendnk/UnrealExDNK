#pragma once

#include "CoreMinimal.h"
#include "TerrainTypes.generated.h"

/** What a terrain actor builds. Picking a type in the editor applies its defaults (ATerrainGenerator::ApplyTypeDefaults). */
UENUM(BlueprintType)
enum class ETerrainType : uint8
{
	/** Land surrounded by sea: island mask on, water on, shallow inland ponds. */
	Island,
	/** Land to the mesh edges: no island mask, no water by default, dry valleys. */
	Continent,
	/** No ground at all, only the water plane (and its collision). */
	WaterOnly,
};

/** How a corridor's surroundings meet its strip. */
UENUM(BlueprintType)
enum class ETerrainCorridorBankMode : uint8
{
	/** Outside the strip, blend back to the raw terrain over Falloff. */
	BlendToRaw,
	/**
	 * Outside the strip, the ground is the strip's profile plus the terrain's own relief (relative to the centre
	 * line), eased in from zero at the strip edge over BankRamp. The surroundings rise and fall with the strip.
	 */
	FollowProfile,
};

/**
 * A strip of shaped ground running along X at a fixed Y, in actor-local space.
 * Inside it the ground follows a smooth profile: clamped to [MinHeight, MaxHeight] above water and slope-limited
 * along X. Outside it blends back to the raw terrain over Falloff, or follows the profile (BankMode). Optionally caps
 * one side's height (e.g. to keep a camera view clear) and sets the island's width around it.
 */
USTRUCT(BlueprintType)
struct TERRAINGENERATOR_API FTerrainCorridor
{
	GENERATED_BODY()

	/** Y of the corridor centre line, local cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor")
	float CenterY = 0.f;

	/** X range the corridor covers, local cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor")
	float StartX = -10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor")
	float EndX = 10000.f;

	/** Half-width (Y) of the fully shaped strip, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor", meta = (ClampMin = "0"))
	float HalfWidth = 1500.f;

	/** Distance over which the shaping blends back into the raw terrain, beyond HalfWidth and beyond StartX/EndX, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor", meta = (ClampMin = "0"))
	float Falloff = 3000.f;

	/** Corridor ground height range above the water level, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor")
	float MinHeight = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor")
	float MaxHeight = 400.f;

	/**
	 * Scale the terrain's own rise and fall along the centre line (over StartX..EndX) into [MinHeight, MaxHeight]
	 * instead of clamping it. Clamping flattens the corridor wherever the terrain is mostly above or below the range;
	 * rescaling keeps its shape, only lower in amplitude.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor")
	bool bRemapToRange = false;

	/**
	 * With bRemapToRange: how many times to rescale and slope-limit. The slope limit only lowers peaks, which shrinks
	 * the span; each further pass stretches it back toward the full range and limits again.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor", meta = (ClampMin = "1", ClampMax = "8", EditCondition = "bRemapToRange"))
	int32 RemapPasses = 1;

	/** Steepest the corridor profile may get along X, degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor", meta = (ClampMin = "0", ClampMax = "89"))
	float MaxSlopeDeg = 10.f;

	/**
	 * With bRemapToRange: after rescaling, lower the whole profile toward MinHeight until this share of its steps
	 * (over StartX..EndX) is within MaxSlopeDeg; the slope limit then trims only the steepest rest. Keeps the hills'
	 * natural shape, so they differ in height and steepness instead of all becoming MaxSlopeDeg ramps up to MaxHeight.
	 * 0 = off (rescale to the full range, then limit).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor", meta = (ClampMin = "0", ClampMax = "1", EditCondition = "bRemapToRange"))
	float SlopeFitShare = 0.f;

	/** Base noise frequency of the profile along X, 1/cm. 0 = the terrain's NoiseScale. Lower = fewer, longer hills. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor", meta = (ClampMin = "0"))
	float ProfileNoiseScale = 0.f;

	/** fBm octaves of the profile. 0 = the terrain's Octaves. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor", meta = (ClampMin = "0", ClampMax = "8"))
	int32 ProfileOctaves = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor|Banks")
	ETerrainCorridorBankMode BankMode = ETerrainCorridorBankMode::BlendToRaw;

	/** FollowProfile: distance past HalfWidth over which the terrain's relief eases in from zero, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor|Banks", meta = (ClampMin = "0", EditCondition = "BankMode == ETerrainCorridorBankMode::FollowProfile"))
	float BankRamp = 3000.f;

	/** FollowProfile: share of the terrain's relief added on the +Y side. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor|Banks", meta = (ClampMin = "0", EditCondition = "BankMode == ETerrainCorridorBankMode::FollowProfile"))
	float BankScalePlus = 1.f;

	/** FollowProfile: share of the terrain's relief added on the -Y side. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor|Banks", meta = (ClampMin = "0", EditCondition = "BankMode == ETerrainCorridorBankMode::FollowProfile"))
	float BankScaleMinus = 1.f;

	/**
	 * FollowProfile: steepest cross slope (across Y) right at the strip edge, degrees; it grows to BankMaxSlopeDeg at
	 * BankRamp. Limits the banks row by row outward from the edge. 0 = no limit.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor|Banks", meta = (ClampMin = "0", ClampMax = "89", EditCondition = "BankMode == ETerrainCorridorBankMode::FollowProfile"))
	float BankEdgeSlopeDeg = 0.f;

	/** FollowProfile with BankEdgeSlopeDeg: steepest cross slope from BankRamp outward, degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor|Banks", meta = (ClampMin = "0", ClampMax = "89", EditCondition = "BankMode == ETerrainCorridorBankMode::FollowProfile"))
	float BankMaxSlopeDeg = 45.f;

	/**
	 * The corridor sets the island's width: within StartX..EndX, dry land runs from CenterY - LandMinusY to
	 * CenterY + LandPlusY (never below MinHeight above water), then a shore drops to the seabed. Island terrain only.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor|Land Width")
	bool bDefineLandWidth = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor|Land Width", meta = (ClampMin = "0", EditCondition = "bDefineLandWidth"))
	float LandPlusY = 4500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor|Land Width", meta = (ClampMin = "0", EditCondition = "bDefineLandWidth"))
	float LandMinusY = 10500.f;

	/** Shore width beyond the +Y land edge, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor|Land Width", meta = (ClampMin = "1", EditCondition = "bDefineLandWidth"))
	float ShorePlusY = 2000.f;

	/** Shore width beyond the -Y land edge, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor|Land Width", meta = (ClampMin = "1", EditCondition = "bDefineLandWidth"))
	float ShoreMinusY = 4000.f;

	/** The corridor profile ignores the island mask, so it stays dry through the shore band. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor")
	bool bOverrideShoreMask = true;

	/**
	 * One-sided height cap: for CenterY + CapFromY < Y <= CenterY + CapToY (signed, so negative values cap the -Y side),
	 * the ground stays at or below the corridor profile + CapClearance. Blends out over CapFalloff beyond CapToY.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor|Side Cap")
	bool bUseSideCap = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor|Side Cap", meta = (EditCondition = "bUseSideCap"))
	float CapFromY = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor|Side Cap", meta = (EditCondition = "bUseSideCap"))
	float CapToY = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor|Side Cap", meta = (EditCondition = "bUseSideCap"))
	float CapClearance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor|Side Cap", meta = (EditCondition = "bUseSideCap", ClampMin = "0"))
	float CapFalloff = 3000.f;
};
