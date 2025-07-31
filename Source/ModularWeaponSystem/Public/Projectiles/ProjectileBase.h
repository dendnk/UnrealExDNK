// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "Types/WeaponTypes.h"
#include "ProjectileBase.generated.h"

class UAudioComponent;
class UInitialActiveSoundParams;
class UNiagaraSystem;
class UProjectileMovementComponent;
class USoundBase;
class UStaticMeshComponent;

/**
 * Base Projectile Class
 */
UCLASS(Abstract, Blueprintable, ClassGroup = (Projectiles), meta = (BlueprintSpawnableComponent))
class MODULARWEAPONSYSTEM_API AProjectileBase : public AActor
{
    GENERATED_BODY()

public:
    AProjectileBase();

    virtual void PostInitProperties() override;

    UFUNCTION(BlueprintCallable, Category = Projectiles)
    virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
    virtual void BeginPlay() override;
    virtual UAudioComponent* CustomSpawnSoundAttached(USoundBase* Sound, USceneComponent* AttachToComponent, FName AttachPointName = NAME_None, FVector Location = FVector(ForceInit), FRotator Rotation = FRotator::ZeroRotator, EAttachLocation::Type LocationType = EAttachLocation::KeepRelativeOffset, bool bStopWhenAttachedToDestroyed = false, float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f, float StartTime = 0.f, USoundAttenuation* AttenuationSettings = nullptr, USoundConcurrency* ConcurrencySettings = nullptr, bool bAutoDestroy = true);
    virtual float CustomApplyDamage(float Damage, AActor* DamageCauser, AActor* OtherActor);
    virtual void CustomPlaySoundAtLocation(const UObject* WorldContextObject, USoundBase* Sound, FVector Location, float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f, float StartTime = 0.f, class USoundAttenuation* AttenuationSettings = nullptr, USoundConcurrency* ConcurrencySettings = nullptr, const UInitialActiveSoundParams* InitialParams = nullptr);

public:
    virtual void Tick(float DeltaTime) override;


    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = BaseRocket)
    TObjectPtr<UProjectileMovementComponent> MovementComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = BaseRocket)
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BaseRocket)
    TObjectPtr<UNiagaraSystem> ShootEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BaseRocket)
    TObjectPtr<UNiagaraSystem> ExplosionEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BaseRocket)
    TObjectPtr<USoundBase> ExplosionSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BaseRocket)
    TObjectPtr<USoundBase> IdleSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BaseRocket)
    FProjectileConfig Config;

private:
    UPROPERTY()
    TObjectPtr<UAudioComponent> IdleAudioComponent;
};
