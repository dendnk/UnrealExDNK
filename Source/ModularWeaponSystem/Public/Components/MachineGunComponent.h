// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "WeaponComponentBase.h"
#include "MachineGunComponent.generated.h"


UCLASS(Blueprintable, ClassGroup = (Weapons), meta = (BlueprintSpawnableComponent))
class MODULARWEAPONSYSTEM_API UMachineGunComponent : public UWeaponComponentBase
{
    GENERATED_BODY()

public:
    UMachineGunComponent();

    virtual void Fire() override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Machine Gun")
    float FireRate;

    FTimerHandle FireTimerHandle;

    void StartFiring();
    void StopFiring();
};
