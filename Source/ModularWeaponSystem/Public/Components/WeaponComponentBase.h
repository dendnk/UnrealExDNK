// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "UI/UserWidgetSpawnerComponent.h"
#include "Data/WeaponDataAsset.h"
#include "WeaponComponentBase.generated.h"

class AProjectileBase;
class UNiagaraSystem;
class USoundBase;
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
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Fire")
    void StartFire();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Fire")
    void StopFire();


    UFUNCTION(BlueprintPure, Category = "Weapon|Data")
    UWeaponDataAsset* GetWeaponDataAsset() const { return WeaponDataAsset; };

    UFUNCTION(BlueprintPure, Category = "Weapon|Data")
    UWeaponDataAsset* GetWeaponDataRuntime() const { return WeaponDataRuntime; };

    UFUNCTION(BlueprintPure, Category = "Weapon|Data")
    int32 GetCurrentAmmo() const { return CurrentAmmo; };

    UFUNCTION(BlueprintCallable, Category = "Weapon|Data")
    void SetCurrentAmmo(int32 NewCurrentAmmo);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Data")
    FTransform GetMuzzleTransform() const;


    UFUNCTION(BlueprintCallable, Category = "Weapon|Projectile")
    TSubclassOf<AProjectileBase> GetProjectileClassByType(const EProjectileType& ProjectileType) const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Projectile")
    void SetProjectileClass(TSubclassOf<AProjectileBase> NewProjectileClass);

    UFUNCTION(BlueprintCallable, Category = "Weapon|Projectile")
    TSubclassOf<AProjectileBase> GetProjectileClass() const { return ProjectileClass; };

    UPROPERTY(BlueprintAssignable, Category = "Weapon|Projectile")
    FOnProjectileClassChangedDelegate OnProjectileClassChanged;


    UFUNCTION(BlueprintPure, Category = "Weapon|UI")
    UWeaponViewModel* GetViewModel() const { return WeaponViewModel; }

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Targets")
    AActor* GetNearestTarget();

    UFUNCTION(BlueprintNativeEvent, Category = "Weapon|FX")
    void SpawnFXAtLocation(UNiagaraSystem* SystemTemplate, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector(1.f), bool bAutoDestroy = true, bool bShouldAutoActivate = true);

    UFUNCTION(BlueprintNativeEvent, Category = "Weapon|FX")
    void PlaySoundAtLocation(USoundBase* Sound, FVector Location, float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f, float StartTime = 0.f);


protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Data", meta = (AllowPrivateAccess))
    TObjectPtr<UWeaponDataAsset> WeaponDataAsset;

    UPROPERTY(Transient, BlueprintReadWrite, Category = "Weapon|Data", meta = (AllowPrivateAccess))
    TObjectPtr<UWeaponDataAsset> WeaponDataRuntime;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Projectile", meta = (AllowPrivateAccess))
    TMap<EProjectileType, TSubclassOf<AProjectileBase>> ProjectileClasses;

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
