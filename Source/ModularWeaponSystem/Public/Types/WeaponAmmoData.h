// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "WeaponAmmoData.generated.h"

USTRUCT(BlueprintType)
struct FWeaponAmmoData
{
	GENERATED_BODY()

	/** Max ammo capacity */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxAmmo = 100;

	/** Ammo consumed per shot */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 AmmoPerShot = 1;

	/** If true, ammo doesn't decrease on firing */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bInfiniteAmmo = false;
};
