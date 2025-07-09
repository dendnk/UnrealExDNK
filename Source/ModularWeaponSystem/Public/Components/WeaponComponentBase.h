// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "WeaponComponentBase.generated.h"


/**
 * Base Weapon Component Class
 */
UCLASS(Abstract, Blueprintable, ClassGroup = (Weapons), meta = (BlueprintSpawnableComponent))
class MODULARWEAPONSYSTEM_API UWeaponComponentBase : public UActorComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, DisplayName = "Fire")
    void BP_Fire();

    virtual void Fire();

    UFUNCTION(BlueprintCallable)
    virtual bool CanFire() const;

protected:
    // Common weapon properties (e.g., cooldown, ammo, etc.)
};
