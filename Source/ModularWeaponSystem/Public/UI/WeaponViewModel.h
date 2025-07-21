// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "MVVMViewModelBase.h"
#include "Types/WeaponTypes.h"
#include "WeaponViewModel.generated.h"

class UWeaponDataAsset;

/**
 *
 */
UCLASS()
class MODULARWEAPONSYSTEM_API UWeaponViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	EFireType FireType;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	EFiringMode FiringMode;

	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//int32 MaxAmmo;
	//
	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//int32 AmmoPerShot;

	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//bool bInfiniteAmmo;

	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//float CooldownTime;

	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//bool bNeedsReload;

	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//float ReloadTime;

	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//float BaseDamage;


	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//float bIsHitscan;

	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//float HitscanRange;

	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//float Spread;


	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//float bIsProjectile;

	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//float ProjectileSpeed;

	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//int32 ProjectilesPerShot;


	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//float bIsBeam;

	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//float BeamDuration;

	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//bool bApplyDoT;

	//UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	//float DamagePerTick;


	//UPROPERTY(BlueprintReadOnly, FieldNotify, Setter, meta = (AllowPrivateAccess))
	//TObjectPtr<UWeaponDataAsset> WeaponData;


public:
	UFUNCTION(BlueprintCallable)
	void InitializeFromWeaponDataAsset(UWeaponDataAsset* DataAsset);

	UFUNCTION(BlueprintCallable)
	void ApplyToWeaponDataAsset(UWeaponDataAsset* DataAsset);


	void SetFireType(EFireType NewFireType);
	EFireType GetFireType() const { return FireType; };

	UFUNCTION(BlueprintPure, FieldNotify)
	FString GetFireTypeAsString() const
	{
		return StaticEnum<EFireType>()->GetNameStringByValue(static_cast<int64>(FireType));
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetFireTypeAsText() const
	{
		return FText::FromString(GetFireTypeAsString());
	}

	void SetFiringMode(EFiringMode NewFiringMode);
	EFiringMode GetFiringMode() const { return FiringMode; };

	UFUNCTION(BlueprintPure, FieldNotify)
	FString GetFiringModeAsString() const
	{
		return StaticEnum<EFiringMode>()->GetNameStringByValue(static_cast<int64>(FiringMode));
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetFiringModeAsText() const
	{
		return FText::FromString(GetFiringModeAsString());
	}

	/*void SetFiringMode(EFiringMode NewFiringMode);
	void SetMaxAmmo(int32 NewMaxAmmo);
	void SetAmmoPerShot(int32 NewAmmoPerShot);
	void SetCurrentAmmo(int32 NewCurrentAmmo);
	void SetIsInfiniteAmmo(bool IsInfiniteAmmo);
	void SetCooldownTime(float NewAmmo);
	void SetBaseDamage(float NewBaseDamage);

	void SetIsHitScan(bool NewAmmo);
	void SetHitscanRange(float NewHitscanRange);
	void SetSpread(bool NewSpread);

	void SetIsProjectile(bool NewIsProjectile);
	void SetProjectileSpeed(float NewProjectileSpeed);
	void SetProjectilesPerShot(int32 NewProjectilesPerShot);

	void SetIsBeam(bool NewIsBeam);
	void SetBeamDuration(float NewBeamDuration);
	void SetIsApplyDoT(bool NewIsApplyDoT);
	void SetDamagePerTick(float NewDamagePerTick);*/
};
