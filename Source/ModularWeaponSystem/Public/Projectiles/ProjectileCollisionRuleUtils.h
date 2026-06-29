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
	DestroyProjectile,
};

USTRUCT(BlueprintType)
struct MODULARWEAPONSYSTEM_API FProjectileCollisionRuleEvaluation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EProjectileCollisionRuleResult Result = EProjectileCollisionRuleResult::NotProjectile;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AProjectileBase> HitProjectile = nullptr;
};

UCLASS()
class MODULARWEAPONSYSTEM_API UProjectileCollisionRuleUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Returns NotProjectile when HitActor is not an AProjectileBase; callers keep their own non-projectile hit flow.
	UFUNCTION(BlueprintPure, Category = "Projectile|Collision")
	static FProjectileCollisionRuleEvaluation EvaluateProjectileCollisionRules(
		const FProjectileCollisionRuleConfig& SourceConfig,
		AActor* HitActor);
};
