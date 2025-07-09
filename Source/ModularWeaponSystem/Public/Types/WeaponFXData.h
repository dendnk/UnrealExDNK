// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "WeaponFXData.generated.h"

class UNiagaraSystem;
class USoundBase;

USTRUCT(BlueprintType)
struct FWeaponFXData
{
	GENERATED_BODY()

	/** Niagara muzzle flash effect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UNiagaraSystem> MuzzleFlashFX = nullptr;

	/** Niagara impact FX */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UNiagaraSystem> ImpactFX = nullptr;

	/** Optional fire sound */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USoundBase> FireSound = nullptr;
};
