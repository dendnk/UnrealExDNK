// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "UIWidgetComponent.h"
#include "Data/WeaponDataAsset.h"
#include "WeaponComponentBase.generated.h"


/**
 * Base Weapon Component Class
 */
UCLASS(Abstract, Blueprintable, ClassGroup = (Weapons), meta = (BlueprintSpawnableComponent))
class MODULARWEAPONSYSTEM_API UWeaponComponentBase : public UUIWidgetComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, DisplayName = "Fire")
    void BP_Fire();

    virtual void Fire();

    UFUNCTION(BlueprintCallable)
    virtual bool CanFire() const;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Config")
    TObjectPtr<UWeaponDataAsset> WeaponData;
};
