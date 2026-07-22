// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "UObject/Interface.h"
#include "IWeaponUserInterface.generated.h"

class UWeaponComponentBase;

// Defines whether a muzzle transform request is read-only or should consume the next sequential muzzle socket.
UENUM(BlueprintType)
enum class EWeaponMuzzleTransformUsage : uint8
{
    Preview,
    Shot
};

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
    /** Returns the world muzzle transform for a weapon/socket. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    FTransform GetMuzzleTransform(UWeaponComponentBase* WeaponComponent, FName MuzzleSocketName, EWeaponMuzzleTransformUsage Usage) const;
    virtual FTransform GetMuzzleTransform_Implementation(UWeaponComponentBase* WeaponComponent, FName MuzzleSocketName, EWeaponMuzzleTransformUsage Usage) const { return FTransform::Identity; }

    /** Allows the weapon owner to block firing when its own aim/state is not ready. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    bool CanFireWeapon(UWeaponComponentBase* WeaponComponent) const;
    virtual bool CanFireWeapon_Implementation(UWeaponComponentBase* WeaponComponent) const { return true; }

    /** Returns the parent component this is attached to, or nullptr if none. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    USceneComponent* GetParentAttachment() const;

    /** Provide access to owner actor */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    AActor* GetWeaponOwner();

    /** Called from weapon animation notifies when the montage reaches its fire frame. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Animation")
    void HandleFireWeaponNotify();
    virtual void HandleFireWeaponNotify_Implementation() {}
};
