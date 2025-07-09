// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "WeaponComponentBase.h"
#include "QuadGunComponent.generated.h"


UCLASS(Blueprintable, ClassGroup = (Weapons), meta = (BlueprintSpawnableComponent))
class MODULARWEAPONSYSTEM_API UQuadGunComponent : public UWeaponComponentBase
{
    GENERATED_BODY()

public:
    UQuadGunComponent();

    virtual void Fire() override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "QuadGun")
    int32 NumberOfBarrels;

    UPROPERTY(EditDefaultsOnly, Category = "QuadGun")
    float BarrelSpreadAngle;
};
