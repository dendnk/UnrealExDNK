// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "WeaponDamageData.generated.h"

class UDamageType;

USTRUCT(BlueprintType)
struct FWeaponDamageData
{
	GENERATED_BODY()

	/** Base damage dealt by the weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseDamage = 25.0f;

	/** Damage type (optional) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UDamageType> DamageType;
};
