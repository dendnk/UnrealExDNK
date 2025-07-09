// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "WeaponReloadData.generated.h"

USTRUCT(BlueprintType)
struct FWeaponReloadData
{
	GENERATED_BODY()

	/** Is reload required */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bNeedsReload = true;

	/** Time required to reload */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ReloadTime = 2.5f;
};
