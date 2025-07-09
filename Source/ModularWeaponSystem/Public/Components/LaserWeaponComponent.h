// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "WeaponComponentBase.h"
#include "LaserWeaponComponent.generated.h"


UCLASS(Blueprintable, ClassGroup = (Weapons), meta = (BlueprintSpawnableComponent))
class UNREALEXDNK_API ULaserWeaponComponent : public UWeaponComponentBase
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
