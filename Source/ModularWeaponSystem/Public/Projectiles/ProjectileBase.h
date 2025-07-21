// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class UAudioComponent;
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

private:
    UPROPERTY()
    TObjectPtr<UAudioComponent> IdleAudioComponent;
};
