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

/**
 * A strip of shaped ground running along X at a fixed Y, in actor-local space.
 * Inside it the ground follows a smooth profile: clamped to [MinHeight, MaxHeight] above water and slope-limited
 * along X. Outside it blends back to the raw terrain over Falloff. Optionally caps one side's height
 * (e.g. to keep a camera view clear).
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

	/** Steepest the corridor profile may get along X, degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor", meta = (ClampMin = "0", ClampMax = "89"))
	float MaxSlopeDeg = 10.f;

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
