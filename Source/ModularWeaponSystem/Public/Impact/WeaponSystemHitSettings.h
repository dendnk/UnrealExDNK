// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "WeaponSystemHitSettings.generated.h"

class UNiagaraSystem;
struct FHitResult;

// Project Settings -> Plugins -> Weapon System Hits: what a bullet hit on the player's pawn shows (HeliAce
// specs/*-vfx-review-findings REQ-1). Replaces the bullet's ExplosionEffect / hitscan ImpactFX on that pawn only.
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Weapon System Hits"))
class MODULARWEAPONSYSTEM_API UWeaponSystemHitSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	static const UWeaponSystemHitSettings* Get() { return GetDefault<UWeaponSystemHitSettings>(); }

	// Bullet (non-AoE projectile or hitscan) hit on a player-controlled pawn. Empty keeps the bullet's own effect.
	UPROPERTY(Config, EditAnywhere, Category = "Player Hits")
	TSoftObjectPtr<UNiagaraSystem> PlayerHitEffect;

	UPROPERTY(Config, EditAnywhere, Category = "Player Hits", meta = (ClampMin = "0.01"))
	float PlayerHitEffectScale = 1.f;

	// Spawns PlayerHitEffect at the hit when Hit's actor is a player-controlled pawn. False (nothing spawned) otherwise.
	static bool TrySpawnPlayerHit(const UObject* WorldContext, const FHitResult& Hit);
};
