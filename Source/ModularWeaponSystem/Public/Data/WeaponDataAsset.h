// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "WeaponType.h"
#include "WeaponDataAsset.generated.h"

class USoundBase;
class UAnimMontage;
class AProjectileBase;

/**
 * Base data asset for defining weapon parameters.
 */
UCLASS(BlueprintType)
class UNREALEXDNK_API UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	/** Type of the weapon (Rocket, Rifle, Shotgun, Laser, etc.) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponType WeaponType;

	/** Class to spawn when firing (for projectile-based weapons) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AProjectileBase> ProjectileClass;

	/** Fire rate in shots per second */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float FireRate = 1.0f;

	/** Base damage this weapon deals */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float Damage = 20.0f;

	/** Max range of hitscan or projectile */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float Range = 10000.0f;

	/** Sound played when firing */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USoundBase> FireSound = nullptr;

	/** Optional animation when firing */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UAnimMontage> FireAnimation = nullptr;

	/** Spread angle (in degrees), for shotguns or inaccurate weapons */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float SpreadAngle = 0.0f;

	/** Number of projectiles per shot (useful for shotguns) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	int32 ProjectilesPerShot = 1;
};
