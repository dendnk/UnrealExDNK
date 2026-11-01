// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "WeaponSystemWaterSettings.generated.h"

class UNiagaraSystem;
class USoundBase;

// Project Settings -> Plugins -> Weapon System Water: the default water impacts (HeliAce
// specs/implementing-water-surface-and-splash REQ-9, REQ-10). A projectile can name its own effect or opt out.
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Weapon System Water"))
class MODULARWEAPONSYSTEM_API UWeaponSystemWaterSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	static const UWeaponSystemWaterSettings* Get() { return GetDefault<UWeaponSystemWaterSettings>(); }

	// Rockets, grenades, flak that explode on the water: the water column.
	UPROPERTY(Config, EditAnywhere, Category = "Projectiles")
	TSoftObjectPtr<UNiagaraSystem> ProjectileWaterEffect;

	// Played in addition to the projectile's own explosion sound. Empty plays nothing.
	UPROPERTY(Config, EditAnywhere, Category = "Projectiles")
	TSoftObjectPtr<USoundBase> ProjectileWaterSound;

	// Projectile bullets and hitscan hits on the water: a small spray puff.
	UPROPERTY(Config, EditAnywhere, Category = "Bullets")
	TSoftObjectPtr<UNiagaraSystem> BulletWaterEffect;

	UPROPERTY(Config, EditAnywhere, Category = "Bullets")
	TSoftObjectPtr<USoundBase> BulletWaterSound;

	// At most one bullet puff per weapon in this many seconds.
	UPROPERTY(Config, EditAnywhere, Category = "Bullets", meta = (ClampMin = "0", Units = "s"))
	float BulletWaterImpactMinInterval = 0.05f;
};
