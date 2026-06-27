// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/WeaponTypes.h"
#include "ProjectileCollisionRuleUtils.generated.h"

class AActor;
class AProjectileBase;

UENUM(BlueprintType)
enum class EProjectileCollisionRuleResult : uint8
{
	NotProjectile,
	RulesDisabled,
	IgnoredByRules,
	DestroyImmediately,
	DamageHealth,
};

USTRUCT(BlueprintType)
struct MODULARWEAPONSYSTEM_API FProjectileCollisionRuleEvaluation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EProjectileCollisionRuleResult Result = EProjectileCollisionRuleResult::NotProjectile;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AProjectileBase> HitProjectile = nullptr;
};

UCLASS()
class MODULARWEAPONSYSTEM_API UProjectileCollisionRuleUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Projectile|Collision")
	static FProjectileCollisionRuleEvaluation EvaluateProjectileCollisionRules(
		const FProjectileCollisionRuleConfig& SourceConfig,
		AActor* HitActor);
};
