// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#include "Projectiles/ProjectileCollisionRuleUtils.h"

#include "Projectiles/ProjectileBase.h"

FProjectileCollisionRuleEvaluation UProjectileCollisionRuleUtils::EvaluateProjectileCollisionRules(
	const FProjectileCollisionRuleConfig& SourceConfig,
	AActor* HitActor)
{
	FProjectileCollisionRuleEvaluation Evaluation;
	AProjectileBase* HitProjectile = Cast<AProjectileBase>(HitActor);
	if (!IsValid(HitProjectile))
	{
		return Evaluation;
	}

	Evaluation.HitProjectile = HitProjectile;

	if (SourceConfig.bEnableProjectileCollisionRules == false)
	{
		Evaluation.Result = EProjectileCollisionRuleResult::RulesDisabled;
		return Evaluation;
	}

	const FProjectileCollisionRuleConfig& HitProjectileConfig = HitProjectile->Config.CollisionRuleConfig;
	if (SourceConfig.ValidTargetProjectileTags.IsEmpty() == false &&
		SourceConfig.ValidTargetProjectileTags.HasTag(HitProjectileConfig.ProjectileTypeTag) == false)
	{
		Evaluation.Result = EProjectileCollisionRuleResult::IgnoredByRules;
		return Evaluation;
	}

	if (SourceConfig.bCanAffectFriendlyProjectiles == false &&
		SourceConfig.ProjectileFactionTag.IsValid() &&
		HitProjectileConfig.ProjectileFactionTag.IsValid() &&
		SourceConfig.ProjectileFactionTag.MatchesTagExact(HitProjectileConfig.ProjectileFactionTag))
	{
		Evaluation.Result = EProjectileCollisionRuleResult::IgnoredByRules;
		return Evaluation;
	}

	Evaluation.Result = EProjectileCollisionRuleResult::DestroyProjectile;

	return Evaluation;
}
