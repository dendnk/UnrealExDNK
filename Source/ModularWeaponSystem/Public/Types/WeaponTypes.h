// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "WeaponTypes.generated.h"

class AActor;
class UDamageType;

namespace WeaponSystemNames
{
	static const FName ProjectileMesh(TEXT("ProjectileMesh"));
	static const FName ProjectileMovement(TEXT("ProjectileMovement"));
}

UENUM(BlueprintType)
enum class EFireType : uint8
{
	// Instant line trace + optional tracer
	Hitscan,

	// Projectile actor
	Projectile,

	// Visible beam FX
	Beam,
};

UENUM(BlueprintType)
enum class EFiringMode : uint8
{
	// Fires once per input press
	SemiAuto,

	// Continues firing while input is held
	FullAuto,

	// Fires a fixed number of shots per input press
	Burst,
};

UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	// Normal rocket, explodes on impact with gravity
	StandardRocket			UMETA(DisplayName = "Standard Rocket"),
	// Normal rocket, explodes on impact no gravity
	StandardRocketNoGravity	UMETA(DisplayName = "Standard Rocket No Gravity"),
	// Cassette-style rocket, splits into smaller ones
	ClusterRocket			UMETA(DisplayName = "Cluster Rocket"),
	// Trajectory deviation (sinusoidal)
	WobbleRocket			UMETA(DisplayName = "Wobble Rocket"),
	// Guided rocket that follows the target
	HomingRocket			UMETA(DisplayName = "Homing Rocket"),
};

USTRUCT(BlueprintType)
struct FDamageData
{
	GENERATED_BODY()

	/** Base damage dealt by the weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseDamage = 25.0f;

	/** Damage over time / damage per tick */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DamagePerTick = 0.f;

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

USTRUCT(BlueprintType)
struct FProjectileConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	float Damage = -1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	float InitialSpeed = -1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	float MaxSpeed = -1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	uint8 bRotationFollowsVelocity : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile, meta = (EditCondition = "bRotationFollowsVelocity"))
	uint8 bRotationRemainsVertical : 1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	uint8 bShouldBounce : 1 = false;
};
