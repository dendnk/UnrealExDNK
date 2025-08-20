// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "UI/UserWidgetSpawnerComponent.h"
#include "Data/WeaponDataAsset.h"
#include "WeaponComponentBase.generated.h"

class AProjectileBase;
class UWeaponViewModel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileClassChangedDelegate, TSubclassOf<AProjectileBase>, NewProjectileClass);


/**
 * Base Weapon Component Class
 */
UCLASS(Abstract, Blueprintable, ClassGroup = (Weapons), meta = (BlueprintSpawnableComponent))
class MODULARWEAPONSYSTEM_API UWeaponComponentBase : public UUserWidgetSpawnerComponent
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

    UFUNCTION()
    void HandleOnWeaponDataPropertyChanged();

public:
    UFUNCTION(BlueprintNativeEvent, DisplayName = "GetMuzzleTransform", Category = "Weapon|Muzzle")
    FTransform BP_GetMuzzleTransform();

    UFUNCTION(BlueprintCallable, DisplayName = "StartFire", Category = "Weapon|Fire")
    void BP_StartFire();

    UFUNCTION(BlueprintCallable, DisplayName = "StopFire", Category = "Weapon|Fire")
    void BP_StopFire();

    UFUNCTION(BlueprintPure, Category = "Weapon|Data")
    UWeaponDataAsset* GetWeaponDataAsset() const { return WeaponDataAsset; };

    UFUNCTION(BlueprintPure, Category = "Weapon|Data")
    UWeaponDataAsset* GetWeaponDataRuntime() const { return WeaponDataRuntime; };

    UFUNCTION(BlueprintPure, Category = "Weapon|Data")
    int32 GetCurrentAmmo() const { return CurrentAmmo; };

    UFUNCTION(BlueprintCallable, Category = "Weapon|Data")
    void SetCurrentAmmo(int32 NewCurrentAmmo);

    UFUNCTION(BlueprintCallable, Category = "Weapon|Projectile")
    TSubclassOf<AProjectileBase> GetProjectileClassByType(const EProjectileType& ProjectileType) const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Projectile")
    void SetProjectileClass(TSubclassOf<AProjectileBase> NewProjectileClass);

    UFUNCTION(BlueprintCallable, Category = "Weapon|Projectile")
    TSubclassOf<AProjectileBase> GetProjectileClass() const { return ProjectileClass; };

    UFUNCTION(BlueprintPure, Category = "Weapon|UI")
    UWeaponViewModel* GetViewModel() const { return WeaponViewModel; }

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    AActor* GetNearestTarget();

    UPROPERTY(BlueprintAssignable, Category = "Weapon|Projectile")
    FOnProjectileClassChangedDelegate OnProjectileClassChanged;


protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Data", meta = (AllowPrivateAccess))
    TObjectPtr<UWeaponDataAsset> WeaponDataAsset;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Projectile", meta = (AllowPrivateAccess))
    TMap<EProjectileType, TSubclassOf<AProjectileBase>> ProjectileClasses;

    UPROPERTY(Transient, BlueprintReadWrite, Category = "Weapon|Data", meta = (AllowPrivateAccess))
    TObjectPtr<UWeaponDataAsset> WeaponDataRuntime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Fire")
    bool bCanFire = true;

    UPROPERTY(Transient)
    TObjectPtr<UWeaponViewModel> WeaponViewModel;


private:
    FTimerHandle FireLoopHandle;
    FTimerHandle BurstHandle;
    int32 CurrentBurstCount = 0;

    UPROPERTY()
    int32 CurrentAmmo;

    UPROPERTY()
    TSubclassOf<AProjectileBase> ProjectileClass;
};
