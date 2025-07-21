// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "Types/WeaponTypes.h"

#include "WeaponDataAsset.generated.h"

class AProjectileBase;
class UWeaponComponentBase;

/**
 * Base data asset for defining weapon parameters.
 */
UCLASS(BlueprintType)
class MODULARWEAPONSYSTEM_API UWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	EFireType FireType = EFireType::Projectile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	EFiringMode FiringMode = EFiringMode::SemiAuto;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 MaxAmmo = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 AmmoPerShot = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bInfiniteAmmo = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float CooldownTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName = "Muzzle";


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Common")
	FDamageData DamageData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Common")
	FReloadData ReloadData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Common")
	FFXData FXData;


	UPROPERTY(VisibleAnywhere, Transient, Category = "Weapon|Condition")
	bool bIsHitscan = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Hitscan", meta = (EditCondition = "bIsHitscan", EditConditionHides = true))
	float HitscanRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Hitscan", meta = (EditCondition = "bIsHitscan", EditConditionHides = true))
	float Spread;


	UPROPERTY(VisibleAnywhere, Transient, Category = "Weapon|Condition")
	bool bIsProjectile = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Projectile", meta = (EditCondition = "bIsProjectile", EditConditionHides = true))
	TSubclassOf<AProjectileBase> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Projectile", meta = (EditCondition = "bIsProjectile", EditConditionHides = true))
	float ProjectileSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Projectile", meta = (EditCondition = "bIsProjectile", EditConditionHides = true))
	int32 ProjectilesPerShot = 1;


	UPROPERTY(VisibleAnywhere, Transient, Category = "Weapon|Condition")
	bool bIsBeam = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Beam", meta = (EditCondition = "bIsBeam", EditConditionHides = true))
	float BeamDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Beam", meta = (EditCondition = "bIsBeam", EditConditionHides = true))
	bool bApplyDoT;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Beam", meta = (EditCondition = "bApplyDoT && bIsBeam", EditConditionHides = true))
	float DamagePerTick;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		static const FName PrimaryAssetType = TEXT("WeaponData");
		return FPrimaryAssetId(PrimaryAssetType, GetFName());
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);

		if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UWeaponDataAsset, FireType))
		{
			bIsHitscan = (FireType == EFireType::Hitscan);
			bIsProjectile = (FireType == EFireType::Projectile);
			bIsBeam = (FireType == EFireType::Beam);
		}
	}
#endif
};
