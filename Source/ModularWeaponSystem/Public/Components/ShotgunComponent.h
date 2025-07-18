// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "WeaponComponentBase.h"
#include "ShotgunComponent.generated.h"


UCLASS(Blueprintable, ClassGroup = (Weapons), meta = (BlueprintSpawnableComponent))
class MODULARWEAPONSYSTEM_API UShotgunComponent : public UWeaponComponentBase
{
    GENERATED_BODY()

public:
    UShotgunComponent();

    virtual void Fire() override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Shotgun")
    int32 PelletCount;

    UPROPERTY(EditDefaultsOnly, Category = "Shotgun")
    float SpreadAngle;
};
