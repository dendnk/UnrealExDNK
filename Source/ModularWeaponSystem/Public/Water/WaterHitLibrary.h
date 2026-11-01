// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "WaterHitLibrary.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UPrimitiveComponent;

// One generic water check (HeliAce specs/implementing-water-surface-and-splash REQ-5). The terrain's water collision
// (ATerrainGenerator::WaterFloor) carries the component tag "Water"; a hit on a component with that tag is a water hit.
UCLASS()
class MODULARWEAPONSYSTEM_API UWaterHitLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static const FName WaterTag;

	UFUNCTION(BlueprintPure, Category = "Water")
	static bool IsWaterComponent(const UPrimitiveComponent* Component);

	UFUNCTION(BlueprintPure, Category = "Water")
	static bool IsWaterHit(const FHitResult& Hit);

	// Location moved onto the water surface: XY kept, Z = the top of the water collision's bounds.
	UFUNCTION(BlueprintPure, Category = "Water")
	static FVector ProjectOntoWaterSurface(const UPrimitiveComponent* WaterComponent, const FVector& Location);

	// A pooled (auto-release), upward-facing one-shot water effect. Returns null if System is null.
	static UNiagaraComponent* SpawnWaterEffect(const UObject* WorldContextObject, UNiagaraSystem* System,
	                                           const FVector& Location);
};
