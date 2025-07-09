// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "WeaponComponentBase.h"
#include "UShotgunComponent.generated.h"


UCLASS(Blueprintable, ClassGroup = (Weapons), meta = (BlueprintSpawnableComponent))
class UNREALEXDNK_API UShotgunComponent : public UWeaponComponentBase
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
