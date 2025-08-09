// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "MVVMViewModelBase.h"
#include "Types/WeaponTypes.h"
#include "UnrealExDNKUtils.h"
#include "WeaponViewModel.generated.h"

class AProjectileBase;;
class UWeaponComponentBase;
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

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	int32 MaxAmmo;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	int32 AmmoPerShot;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	int32 CurrentAmmo;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = SetIsInfiniteAmmo, Getter = IsInfiniteAmmo, meta = (AllowPrivateAccess))
	bool bInfiniteAmmo;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	int32 BurstCount;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	float CooldownTime;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = SetIsNeedReload, Getter = IsNeedReload, meta = (AllowPrivateAccess))
	bool bNeedsReload;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	float ReloadTime;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	float BaseDamage;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	float DamagePerTick;


	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	float HitscanRange;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	float HitscanSpread;


	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	float ProjectileSpeed;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	EProjectileType ProjectileType;


	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	float BeamDuration;

	//UPROPERTY(BlueprintReadOnly, FieldNotify, Setter, meta = (AllowPrivateAccess))
	//TObjectPtr<UWeaponDataAsset> WeaponData;


public:
	UFUNCTION(BlueprintCallable)
	void InitializeFromWeapon(UWeaponComponentBase* InWeapon);

	UFUNCTION(BlueprintCallable)
	void ApplyToCurrentWeapon();

	UFUNCTION(BlueprintCallable)
	void Deinit();


private:
	void InitializeFromWeaponDataAsset(UWeaponDataAsset* DataAsset);
	void ApplyToWeaponDataAsset(UWeaponDataAsset* DataAsset);


public:
	void SetFireType(EFireType NewFireType);
	EFireType GetFireType() const { return FireType; };

	UFUNCTION(BlueprintPure, FieldNotify)
	FString GetFireTypeAsString() const
	{
		return UUnrealExDNKUtils::GetEnumAsString(FireType);
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
		return UUnrealExDNKUtils::GetEnumAsString(FiringMode);
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetFiringModeAsText() const
	{
		return FText::FromString(GetFiringModeAsString());
	}

	void SetMaxAmmo(int32 NewMaxAmmo);
	int32 GetMaxAmmo() const { return MaxAmmo; };

	void SetAmmoPerShot(int32 NewAmmoPerShot);
	int32 GetAmmoPerShot() const { return AmmoPerShot; };

	void SetCurrentAmmo(int32 NewCurrentAmmo);
	int32 GetCurrentAmmo() const { return CurrentAmmo; };

	void SetIsInfiniteAmmo(bool NewIsInfiniteAmmo);
	bool IsInfiniteAmmo() const { return bInfiniteAmmo; };

	void SetBurstCount(int32 NewBurstCount);
	int32 GetBurstCount() const { return BurstCount; };

	void SetCooldownTime(float NewCooldownTime);
	float GetCooldownTime() const { return CooldownTime; };

	void SetIsNeedReload(bool NewIsNeedReload);
	bool IsNeedReload() const { return bNeedsReload; };

	void SetReloadTime(float NewReloadTime);
	float GetReloadTime() const { return ReloadTime; };

	void SetBaseDamage(float NewBaseDamage);
	float GetBaseDamage() const { return BaseDamage; };

	void SetDamagePerTick(float NewDamagePerTick);
	float GetDamagePerTick() const { return DamagePerTick; };

	void SetHitscanRange(float NewHitscanRange);
	float GetHitscanRange() const { return HitscanRange; };

	void SetHitscanSpread(float NewHitscanSpread);
	float GetHitscanSpread() const { return HitscanSpread; };

	void SetProjectileType(EProjectileType NewProjectileType);
	EProjectileType GetProjectileType() const { return ProjectileType; };

	UFUNCTION(BlueprintPure, FieldNotify)
	FString GetProjectileTypeAsString() const
	{
		return UUnrealExDNKUtils::GetEnumAsString(ProjectileType);
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetProjectileTypeAsText() const
	{
		return FText::FromString(GetProjectileTypeAsString());
	}

	void SetProjectileSpeed(float NewProjectileSpeed);
	float GetProjectileSpeed() const { return ProjectileSpeed; };

	void SetBeamDuration(float NewBeamDuration);
	float GetBeamDuration() const { return BeamDuration; };

private:
	TWeakObjectPtr<UWeaponComponentBase> Weapon;
};
