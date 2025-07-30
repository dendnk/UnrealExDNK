// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "UIWidgetComponent.h"
#include "Data/WeaponDataAsset.h"
#include "WeaponComponentBase.generated.h"

class UWeaponViewModel;

/**
 * Base Weapon Component Class
 */
UCLASS(Abstract, Blueprintable, ClassGroup = (Weapons), meta = (BlueprintSpawnableComponent))
class MODULARWEAPONSYSTEM_API UWeaponComponentBase : public UUIWidgetComponent
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

    virtual void InitWeaponData();

public:
    virtual void StartFire();
    virtual void StopFire();
    virtual void HandleBurstFire();
    virtual void SetupSpawnedProjectile(AProjectileBase* SpawnedProjectile);

private:
    virtual void Fire();
    virtual void FireProjectile();
    virtual void FireHitscan();
    virtual void FireBeam();

public:
    UFUNCTION(BlueprintNativeEvent, DisplayName = "GetMuzzleTransform", Category = "Weapon|Muzzle")
    FTransform BP_GetMuzzleTransform();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, DisplayName = "Fire", Category = "Weapon|Fire")
    void BP_Fire();

    UFUNCTION(BlueprintPure, Category = "Weapon|Data")
    UWeaponDataAsset* GetWeaponDataAsset() const { return WeaponDataAsset; };

    UFUNCTION(BlueprintPure, Category = "Weapon|Data")
    UWeaponDataAsset* GetWeaponDataRuntime() const { return WeaponDataRuntime; };

    UFUNCTION(BlueprintPure, Category = "Weapon|Data")
    int32 GetCurrentAmmo() const { return CurrentAmmo; };

    UFUNCTION(BlueprintCallable, Category = "Weapon|Data")
    void SetCurrentAmmo(int32 NewCurrentAmmo);

    UFUNCTION(BlueprintPure, Category = "Weapon|UI")
    UWeaponViewModel* GetViewModel() const { return WeaponViewModel; }


protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Data", meta = (AllowPrivateAccess))
    TObjectPtr<UWeaponDataAsset> WeaponDataAsset;

    UPROPERTY(Transient, BlueprintReadWrite, Category = "Weapon|Data", meta = (AllowPrivateAccess))
    TObjectPtr<UWeaponDataAsset> WeaponDataRuntime;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon|Data", meta = (AllowPrivateAccess))
    int32 CurrentAmmo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Fire")
    bool bCanFire = true;

    UPROPERTY(Transient)
    TObjectPtr<UWeaponViewModel> WeaponViewModel;


private:
    FTimerHandle FireLoopHandle;
    FTimerHandle BurstHandle;
    int32 CurrentBurstCount = 0;
};
