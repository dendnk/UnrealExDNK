// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "WeaponComponentBase.generated.h"


/**
 * Base Weapon Component Class
 */
UCLASS(Abstract, Blueprintable, ClassGroup = (Weapons), meta = (BlueprintSpawnableComponent))
class UNREALEXDNK_API UWeaponComponentBase : public UActorComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent)
    void Fire();

    virtual void Fire_Implementation();

    UFUNCTION(BlueprintCallable)
    virtual bool CanFire() const;

protected:
    // Common weapon properties (e.g., cooldown, ammo, etc.)
};
