// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.


#include "UI/WeaponViewModel.h"
#include "Components/WeaponComponentBase.h"
#include "Data/WeaponDataAsset.h"
#include "UnrealExDNKUtils.h"

void UWeaponViewModel::InitializeFromWeapon(UWeaponComponentBase* InWeapon)
{
	if (IsValid(InWeapon) == false)
	{
		UE_DNK_LOG(LogTemp, Error, "Invalid Weapon!");
		return;
	}

	Weapon = InWeapon;
	InitializeFromWeaponDataAsset(Weapon->GetWeaponDataRuntime());
	SetCurrentAmmo(Weapon->GetCurrentAmmo());
}

void UWeaponViewModel::ApplyToCurrentWeapon()
{
	if (Weapon.IsValid() == false)
	{
		UE_DNK_LOG(LogTemp, Error, "Invalid Weapon!");
		return;
	}

	ApplyToWeaponDataAsset(Weapon->GetWeaponDataRuntime());
	Weapon->SetCurrentAmmo(GetCurrentAmmo());
}

void UWeaponViewModel::Deinit()
{
	Weapon.Reset();
}

void UWeaponViewModel::InitializeFromWeaponDataAsset(UWeaponDataAsset* DataAsset)
{
	if (IsValid(DataAsset) == false)
	{
		UE_DNK_LOG(LogTemp, Error, "Invalid DataAsset!");
		return;
	}

	SetFireType(DataAsset->FireType);
	SetFiringMode(DataAsset->FiringMode);
	SetMaxAmmo(DataAsset->MaxAmmo);
	SetAmmoPerShot(DataAsset->AmmoPerShot);
	SetIsInfiniteAmmo(DataAsset->bInfiniteAmmo);
	SetBurstCount(DataAsset->BurstCount);
	SetCooldownTime(DataAsset->CooldownTime);
	SetIsNeedReload(DataAsset->ReloadData.bNeedsReload);
	SetReloadTime(DataAsset->ReloadData.ReloadTime);
	SetBaseDamage(DataAsset->DamageData.BaseDamage);
	SetDamagePerTick(DataAsset->DamageData.DamagePerTick);
	SetHitscanRange(DataAsset->HitscanRange);
	SetHitscanSpread(DataAsset->HitscanSpread);
	SetProjectileSpeed(DataAsset->ProjectileSpeed);
	SetBeamDuration(DataAsset->BeamDuration);
}

void UWeaponViewModel::ApplyToWeaponDataAsset(UWeaponDataAsset* DataAsset)
{
	if (IsValid(DataAsset) == false)
	{
		UE_DNK_LOG(LogTemp, Error, "Invalid DataAsset!");
		return;
	}

	DataAsset->FireType = GetFireType();
	DataAsset->FiringMode = GetFiringMode();
	DataAsset->MaxAmmo = GetMaxAmmo();
	DataAsset->AmmoPerShot = GetAmmoPerShot();
	DataAsset->bInfiniteAmmo = IsInfiniteAmmo();
	DataAsset->BurstCount = GetBurstCount();
	DataAsset->CooldownTime = GetCooldownTime();
	DataAsset->ReloadData.bNeedsReload = IsNeedReload();
	DataAsset->ReloadData.ReloadTime = GetReloadTime();
	DataAsset->DamageData.BaseDamage = GetBaseDamage();
	DataAsset->DamageData.DamagePerTick = GetDamagePerTick();
	DataAsset->HitscanRange = GetHitscanRange();
	DataAsset->HitscanSpread = GetHitscanSpread();
	DataAsset->ProjectileSpeed = GetProjectileSpeed();
	DataAsset->BeamDuration = GetBeamDuration();
}

void UWeaponViewModel::SetFireType(EFireType NewFireType)
{
	UE_MVVM_SET_PROPERTY_VALUE(FireType, NewFireType);
}

void UWeaponViewModel::SetFiringMode(EFiringMode NewFiringMode)
{
	UE_MVVM_SET_PROPERTY_VALUE(FiringMode, NewFiringMode);
}

void UWeaponViewModel::SetMaxAmmo(int32 NewMaxAmmo)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxAmmo, NewMaxAmmo);
}

void UWeaponViewModel::SetAmmoPerShot(int32 NewAmmoPerShot)
{
	UE_MVVM_SET_PROPERTY_VALUE(AmmoPerShot, NewAmmoPerShot);
}

void UWeaponViewModel::SetCurrentAmmo(int32 NewCurrentAmmo)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentAmmo, NewCurrentAmmo);
}

void UWeaponViewModel::SetIsInfiniteAmmo(bool NewIsInfiniteAmmo)
{
	UE_MVVM_SET_PROPERTY_VALUE(bInfiniteAmmo, NewIsInfiniteAmmo);
}

void UWeaponViewModel::SetBurstCount(int32 NewBurstCount)
{
	UE_MVVM_SET_PROPERTY_VALUE(BurstCount, NewBurstCount);
}

void UWeaponViewModel::SetCooldownTime(float NewCooldownTime)
{
	UE_MVVM_SET_PROPERTY_VALUE(CooldownTime, NewCooldownTime);
}

void UWeaponViewModel::SetIsNeedReload(bool NewIsNeedReload)
{
	UE_MVVM_SET_PROPERTY_VALUE(bNeedsReload, NewIsNeedReload);
}

void UWeaponViewModel::SetReloadTime(float NewReloadTime)
{
	UE_MVVM_SET_PROPERTY_VALUE(ReloadTime, NewReloadTime);
}

void UWeaponViewModel::SetBaseDamage(float NewBaseDamage)
{
	UE_MVVM_SET_PROPERTY_VALUE(BaseDamage, NewBaseDamage);
}

void UWeaponViewModel::SetDamagePerTick(float NewDamagePerTick)
{
	UE_MVVM_SET_PROPERTY_VALUE(DamagePerTick, NewDamagePerTick);
}

void UWeaponViewModel::SetHitscanRange(float NewHitscanRange)
{
	UE_MVVM_SET_PROPERTY_VALUE(HitscanRange, NewHitscanRange);
}

void UWeaponViewModel::SetHitscanSpread(float NewHitscanSpread)
{
	UE_MVVM_SET_PROPERTY_VALUE(HitscanSpread, NewHitscanSpread);
}

void UWeaponViewModel::SetProjectileSpeed(float NewProjectileSpeed)
{
	UE_MVVM_SET_PROPERTY_VALUE(ProjectileSpeed, NewProjectileSpeed);
}

void UWeaponViewModel::SetBeamDuration(float NewBeamDuration)
{
	UE_MVVM_SET_PROPERTY_VALUE(BeamDuration, NewBeamDuration);
}
