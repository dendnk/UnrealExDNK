// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "GameFramework/DamageType.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "WeaponTypes.generated.h"

class AActor;
class UDamageType;

UENUM(BlueprintType)
enum class EFireType : uint8
{
	Hitscan,	// Instant line trace + optional tracer
	Projectile,	// Projectile actor
	Beam,		// Visible beam FX
};

UENUM(BlueprintType)
enum class EFiringMode : uint8
{
	SemiAuto,	// Fires once per input press
	FullAuto,	// Continues firing while input is held
	Burst,		// Fires a fixed number of shots per input press
};


USTRUCT(BlueprintType)
struct FDamageData
{
	GENERATED_BODY()

	/** Base damage dealt by the weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseDamage = 25.0f;

	/** Damage type (optional) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UDamageType> DamageType;
};

USTRUCT(BlueprintType)
struct FFXData
{
	GENERATED_BODY()

	/**
	 * Niagara system to spawn at the muzzle when firing.
	 * Usually attached to a socket at the muzzle tip.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UNiagaraSystem> MuzzleFlashFX = nullptr;

	/** Niagara impact FX */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UNiagaraSystem> ImpactFX = nullptr;

	/** Optional fire sound */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USoundBase> FireSound = nullptr;
};

USTRUCT(BlueprintType)
struct FReloadData
{
	GENERATED_BODY()

	/** Is reload required */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bNeedsReload = true;

	/** Time required to reload */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ReloadTime = 2.5f;
};
