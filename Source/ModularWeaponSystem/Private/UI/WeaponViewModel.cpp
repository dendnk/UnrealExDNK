// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.


#include "UI/WeaponViewModel.h"
#include "Data/WeaponDataAsset.h"
#include "UnrealExDNKUtils.h"

void UWeaponViewModel::InitializeFromWeaponDataAsset(UWeaponDataAsset* DataAsset)
{
	if (IsValid(DataAsset) == false)
	{
		UE_DNK_LOG(LogTemp, Error, "Invalid DataAsset!");
		return;
	}

	SetFireType(DataAsset->FireType);
	SetFiringMode(DataAsset->FiringMode);
}

void UWeaponViewModel::ApplyToWeaponDataAsset(UWeaponDataAsset* DataAsset)
{
	if (IsValid(DataAsset))
	{
		UE_DNK_LOG(LogTemp, Error, "Invalid DataAsset!");
		return;
	}

	DataAsset->FireType = GetFireType();
	DataAsset->FiringMode = GetFiringMode();
}

void UWeaponViewModel::SetFireType(EFireType NewFireType)
{
	UE_MVVM_SET_PROPERTY_VALUE(FireType, NewFireType);
}

void UWeaponViewModel::SetFiringMode(EFiringMode NewFiringMode)
{
	UE_MVVM_SET_PROPERTY_VALUE(FiringMode, NewFiringMode);
}

//void UWeaponViewModel::SetMaxAmmo(int32 NewMaxAmmo)
//{
//}
//
//void UWeaponViewModel::SetAmmoPerShot(int32 NewAmmoPerShot)
//{
//}
//
//void UWeaponViewModel::SetCurrentAmmo(int32 NewCurrentAmmo)
//{
//}
//
//void UWeaponViewModel::SetIsInfiniteAmmo(bool IsInfiniteAmmo)
//{
//}
//
//void UWeaponViewModel::SetCooldownTime(float NewAmmo)
//{
//}
//
//void UWeaponViewModel::SetBaseDamage(float NewBaseDamage)
//{
//}
//
//void UWeaponViewModel::SetIsHitScan(bool NewAmmo)
//{
//}
//
//void UWeaponViewModel::SetHitscanRange(float NewHitscanRange)
//{
//}
//
//void UWeaponViewModel::SetSpread(bool NewSpread)
//{
//}
//
//void UWeaponViewModel::SetIsProjectile(bool NewIsProjectile)
//{
//}
//
//void UWeaponViewModel::SetProjectileSpeed(float NewProjectileSpeed)
//{
//}
//
//void UWeaponViewModel::SetProjectilesPerShot(int32 NewProjectilesPerShot)
//{
//}
//
//void UWeaponViewModel::SetIsBeam(bool NewIsBeam)
//{
//}
//
//void UWeaponViewModel::SetBeamDuration(float NewBeamDuration)
//{
//}
//
//void UWeaponViewModel::SetIsApplyDoT(bool NewIsApplyDoT)
//{
//}
//
//void UWeaponViewModel::SetDamagePerTick(float NewDamagePerTick)
//{
//}
