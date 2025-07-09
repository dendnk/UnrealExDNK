// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "WeaponFiringData.generated.h"

class AActor;

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
