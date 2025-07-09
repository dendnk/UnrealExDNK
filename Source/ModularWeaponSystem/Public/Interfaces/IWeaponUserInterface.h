// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "UObject/Interface.h"
#include "IWeaponUserInterface.generated.h"

UINTERFACE(Blueprintable)
class UNREALEXDNK_API UWeaponUserInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface for actors that can use weapons.
 */
class UNREALEXDNK_API IWeaponUserInterface
{
    GENERATED_BODY()

public:
    /** Returns the world location of the firing point */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    FVector GetMuzzleLocation() const;

    /** Returns the aim direction */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    FVector GetAimDirection() const;

    /** Optionally return the socket name or bone where the weapon attaches */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    FName GetWeaponSocketName() const;

    /** Called when the weapon starts firing */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    void OnWeaponFired();

    /** Optional: Provide access to owner actor */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    AActor* GetWeaponOwner() const;
};
