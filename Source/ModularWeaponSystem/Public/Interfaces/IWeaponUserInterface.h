// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "UObject/Interface.h"
#include "IWeaponUserInterface.generated.h"

UINTERFACE(Blueprintable)
class MODULARWEAPONSYSTEM_API UWeaponUserInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface for actors that can use weapons.
 */
class MODULARWEAPONSYSTEM_API IWeaponUserInterface
{
    GENERATED_BODY()

public:
    /** Returns the world transform */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    FTransform GetMuzzleTransform() const;

    /** Returns the parent component this is attached to, or nullptr if none. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    USceneComponent* GetParentAttachment() const;

    /** Provide access to owner actor */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    AActor* GetWeaponOwner() const;
};
