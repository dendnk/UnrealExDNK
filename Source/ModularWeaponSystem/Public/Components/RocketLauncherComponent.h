// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "WeaponComponentBase.h"
#include "RocketLauncherComponent.generated.h"


/**
 * Rocket launcher component that fires projectile-based rockets.
 * Inherits from UWeaponComponentBase for modular weapon behavior.
 */
UCLASS(Blueprintable, ClassGroup = (Weapons), meta = (BlueprintSpawnableComponent))
class MODULARWEAPONSYSTEM_API URocketLauncherComponent : public UWeaponComponentBase
{
    GENERATED_BODY()

public:
    URocketLauncherComponent();

    virtual void Fire() override;

protected:
    virtual void BeginPlay() override;

    void SpawnProjectile();
};
