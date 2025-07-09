// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "WeaponComponentBase.h"
#include "LaserWeaponComponent.generated.h"


UCLASS(Blueprintable, ClassGroup = (Weapons), meta = (BlueprintSpawnableComponent))
class MODULARWEAPONSYSTEM_API ULaserWeaponComponent : public UWeaponComponentBase
{
    GENERATED_BODY()

public:
    ULaserWeaponComponent();

    virtual void Fire() override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Laser")
    float BeamLength;

    UPROPERTY(EditDefaultsOnly, Category = "Laser")
    float DamagePerSecond;

    void DealLaserDamage();
};
