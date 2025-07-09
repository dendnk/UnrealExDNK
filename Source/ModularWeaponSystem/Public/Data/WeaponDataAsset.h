// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "WeaponTypes.h"

#include "WeaponDataAsset.generated.h"

/**
 * Base data asset for defining weapon parameters.
 */
UCLASS(BlueprintType)
class UNREALEXDNK_API UWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/** Type of the weapon (Rocket, Rifle, Shotgun, Laser, etc.) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponType WeaponType;

	/** Class of weapon component that implements this weapon's behavior */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class UWeaponComponent> WeaponComponentClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Firing")
	FWeaponFiringData Firing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	FWeaponDamageData Damage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	FWeaponAmmoData Ammo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reload")
	FWeaponReloadData Reload;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	FWeaponFXData FX;


	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		static const FName PrimaryAssetType = TEXT("WeaponData");
		return FPrimaryAssetId(PrimaryAssetType, GetFName());
	}
};
