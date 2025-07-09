// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "GameFramework/DamageType.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "WeaponTypes.generated.h"

class AActor;
class UDamageType;

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	RocketLauncher		UMETA(DisplayName = "RocketLauncher"),
	MachineGun			UMETA(DisplayName = "MachineGun"),
	Quadgun				UMETA(DisplayName = "Quadgun"),
	Shotgun				UMETA(DisplayName = "Shotgun"),
	Laser				UMETA(DisplayName = "Laser"),
	Custom				UMETA(DisplayName = "Custom")
};

USTRUCT(BlueprintType)
struct FWeaponAmmoData
{
	GENERATED_BODY()

	/** Max ammo capacity */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxAmmo = 100;

	/** Ammo consumed per shot */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 AmmoPerShot = 1;

	/** If true, ammo doesn't decrease on firing */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bInfiniteAmmo = false;
};

USTRUCT(BlueprintType)
struct FWeaponDamageData
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
struct FWeaponFiringData
{
	GENERATED_BODY()

	/** Cooldown between shots (in seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CooldownTime = 0.1f;

	/** Does the weapon support automatic fire? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bAutomaticFire = true;

	/** How many projectiles per shot (e.g., for shotguns) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 ProjectilesPerShot = 1;

	/** Angle (degrees) of spread for random deviation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BulletSpreadAngle = 1.0f;

	/** Projectile class to spawn (for rockets, grenades, etc.) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AActor> ProjectileClass;

	/** Muzzle socket name to spawn projectiles or FX */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName MuzzleSocketName = "Muzzle";
};

USTRUCT(BlueprintType)
struct FWeaponFXData
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
struct FWeaponReloadData
{
	GENERATED_BODY()

	/** Is reload required */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bNeedsReload = true;

	/** Time required to reload */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ReloadTime = 2.5f;
};
