// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "WeaponTypes.generated.h"

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
	// Lobbed explosive, arcs under gravity, explodes on any hit (ground, helicopter, etc.)
	Grenade					UMETA(DisplayName = "Grenade"),
	// Flies flat (no gravity), explodes at the target's position when fired, or on direct hit
	FlackProjectile			UMETA(DisplayName = "Flack Projectile"),
	// Flies flat (no gravity), single-target, explodes on direct hit; silently despawns with no FX/SFX if it reaches its lifespan without hitting anything
	Bullet					UMETA(DisplayName = "Bullet"),
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

	/** Volume multiplier applied when playing FireSound. Lets a weapon reuse another weapon's fire sound at a different loudness. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float FireSoundVolumeMultiplier = 1.f;
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
struct FProjectileCollisionRuleConfig
{
	GENERATED_BODY()

	/** Enables custom projectile-vs-projectile collision handling for this projectile or weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile|Collision")
	bool bEnableProjectileCollisionRules = false;

	/** Gameplay tag that identifies this projectile type for collision filtering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile|Collision")
	FGameplayTag ProjectileTypeTag;

	/** Gameplay tag that identifies this projectile faction/team for friendly-fire filtering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile|Collision")
	FGameplayTag ProjectileFactionTag;

	/** Allows this projectile or weapon to affect projectiles with the same faction tag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile|Collision")
	bool bCanAffectFriendlyProjectiles = false;

	/** If not empty, only projectiles with one of these type tags can be affected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile|Collision")
	FGameplayTagContainer ValidTargetProjectileTags;

	/** Destroys this projectile after it successfully affects another projectile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile|Collision")
	bool bConsumeSelfOnProjectileCollision = true;
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

	/** When true, skip spawning ExplosionEffect/ExplosionSound specifically when this projectile's lifespan expires (a miss). A direct hit still explodes with FX/SFX regardless of this flag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	uint8 bSuppressExplosionFxOnLifespanExpiry : 1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Collision")
	FProjectileCollisionRuleConfig CollisionRuleConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|AoE")
	uint8 bHasAoEOnExplode : 1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|AoE", meta = (EditCondition = "bHasAoEOnExplode", ClampMin = "0.0", Units = "cm"))
	float AoERadius = 0.f;

	/** How long the AoE radius debug sphere stays visible after explosion. Match this to the ExplosionEffect's playback length. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|AoE", meta = (EditCondition = "bHasAoEOnExplode", ClampMin = "0.0", Units = "s"))
	float AoEVisualizationDuration = 0.5f;

	/** Safety net: if the projectile stops making progress for this long without exploding (e.g. it collided with something its collision rules ignored), it silently disappears instead of sitting frozen forever. 0 disables the check. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Failsafe", meta = (ClampMin = "0.0", Units = "s"))
	float StuckFailsafeSeconds = 0.35f;
};
