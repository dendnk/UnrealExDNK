// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "UObject/Interface.h"
#include "IWeaponUserInterface.generated.h"

class UWeaponComponentBase;

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

    /** Allows the weapon owner to resolve a muzzle transform for a specific weapon/socket. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    bool TryResolveWeaponMuzzleTransform(UWeaponComponentBase* WeaponComponent, FName MuzzleSocketName, UPARAM(ref) FTransform& OutMuzzleTransform) const;
    virtual bool TryResolveWeaponMuzzleTransform_Implementation(UWeaponComponentBase* WeaponComponent, FName MuzzleSocketName, FTransform& OutMuzzleTransform) const { return false; }

    /** Allows the weapon owner to consume a muzzle transform for an actual shot. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    bool TryResolveWeaponMuzzleTransformForShot(UWeaponComponentBase* WeaponComponent, FName MuzzleSocketName, UPARAM(ref) FTransform& OutMuzzleTransform) const;
    virtual bool TryResolveWeaponMuzzleTransformForShot_Implementation(UWeaponComponentBase* WeaponComponent, FName MuzzleSocketName, FTransform& OutMuzzleTransform) const
    {
        return false;
    }

    /** Allows the weapon owner to block firing when its own aim/state is not ready. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    bool CanFireWeapon(UWeaponComponentBase* WeaponComponent) const;
    virtual bool CanFireWeapon_Implementation(UWeaponComponentBase* WeaponComponent) const { return true; }

    /** Returns the parent component this is attached to, or nullptr if none. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    USceneComponent* GetParentAttachment() const;

    /** Provide access to owner actor */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    AActor* GetWeaponOwner() const;
};
