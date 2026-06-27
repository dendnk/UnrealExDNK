// Copyright 2024 [HeliAce: Denis Kruchok, Denis Romanovsky]. All rights reserved.


#include "Projectiles/ProjectileBase.h"

#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Projectiles/ProjectileCollisionRuleUtils.h"
#include "Types/WeaponTypes.h"


AProjectileBase::AProjectileBase()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(WeaponSystemNames::ProjectileMesh);
    RootComponent = MeshComponent;

    MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(WeaponSystemNames::ProjectileMovement);

    MeshComponent->OnComponentHit.AddDynamic(this, &AProjectileBase::OnProjectileHit);
}

void AProjectileBase::PostInitProperties()
{
    Super::PostInitProperties();

    if (IsValid(MovementComponent))
    {
        MovementComponent->InitialSpeed = Config.InitialSpeed;
        MovementComponent->MaxSpeed = Config.MaxSpeed;
        MovementComponent->bRotationFollowsVelocity = Config.bRotationFollowsVelocity;
        MovementComponent->bRotationRemainsVertical = Config.bRotationRemainsVertical;
        MovementComponent->bShouldBounce = Config.bShouldBounce;
    }
}

void AProjectileBase::BeginPlay()
{
    Super::BeginPlay();

    IdleAudioComponent = CustomSpawnSoundAttached(IdleSound, MeshComponent, NAME_None, FVector(ForceInit), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true, 1.f, 1.f, 0.f, nullptr, nullptr, false);

}

UAudioComponent* AProjectileBase::CustomSpawnSoundAttached(USoundBase* Sound, USceneComponent* AttachToComponent, FName AttachPointName, FVector Location, FRotator Rotation, EAttachLocation::Type LocationType, bool bStopWhenAttachedToDestroyed, float VolumeMultiplier, float PitchMultiplier, float StartTime, USoundAttenuation* AttenuationSettings, USoundConcurrency* ConcurrencySettings, bool bAutoDestroy)
{
    return UGameplayStatics::SpawnSoundAttached(Sound, AttachToComponent, AttachPointName, Location, LocationType, bStopWhenAttachedToDestroyed, VolumeMultiplier, PitchMultiplier, StartTime, AttenuationSettings, ConcurrencySettings, bAutoDestroy);
}

float AProjectileBase::CustomApplyDamage(float Damage, AActor* DamageCauser, AActor* OtherActor)
{
    AController* InstigatorController = DamageCauser != nullptr
                                        ? DamageCauser->GetInstigatorController()
                                        : nullptr;
    return UGameplayStatics::ApplyDamage(OtherActor, Damage, InstigatorController, DamageCauser, nullptr);
}

void AProjectileBase::CustomPlaySoundAtLocation(const UObject* WorldContextObject, USoundBase* Sound, FVector Location, float VolumeMultiplier, float PitchMultiplier, float StartTime, USoundAttenuation* AttenuationSettings, USoundConcurrency* ConcurrencySettings, const UInitialActiveSoundParams* InitialParams)
{
    return UGameplayStatics::PlaySoundAtLocation(WorldContextObject, Sound, Location, VolumeMultiplier, PitchMultiplier, StartTime, AttenuationSettings, ConcurrencySettings, InitialParams);
}

void AProjectileBase::LifeSpanExpired()
{
    ExplodeProjectile(FHitResult());
}

void AProjectileBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AProjectileBase::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (bIsAlreadyExploded)
    {
        return;
    }

    if (IsValid(OtherActor) == false ||
        OtherActor == this ||
        OtherActor == GetOwner())
    {
        return;
    }

    HandleProjectileCollisionHit(OtherActor, Hit);
}

void AProjectileBase::ExplodeProjectile(const FHitResult& Hit)
{
    if (bIsAlreadyExploded)
    {
        return;
    }

    bIsAlreadyExploded = true;
    SetActorEnableCollision(false);

    if (ExplosionEffect != nullptr)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ExplosionEffect, Hit.Location);
    }

    if (IdleAudioComponent != nullptr)
    {
        IdleAudioComponent->Stop();
    }

    if (ExplosionSound != nullptr)
    {
        CustomPlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
    }

    Destroy();
}

void AProjectileBase::HandleProjectileCollisionHit(AActor* HitActor, const FHitResult& Hit)
{
    if (!IsValid(HitActor))
    {
        return;
    }

    const FProjectileCollisionRuleEvaluation Evaluation =
        UProjectileCollisionRuleUtils::EvaluateProjectileCollisionRules(Config.CollisionRuleConfig, HitActor);

    switch (Evaluation.Result)
    {
    case EProjectileCollisionRuleResult::NotProjectile:
    case EProjectileCollisionRuleResult::RulesDisabled:
        CustomApplyDamage(Config.Damage, this, HitActor);
        ExplodeProjectile(Hit);
        return;

    case EProjectileCollisionRuleResult::IgnoredByRules:
        return;

    case EProjectileCollisionRuleResult::DestroyImmediately:
        Evaluation.HitProjectile->ExplodeProjectile(Hit);
        break;

    case EProjectileCollisionRuleResult::DamageHealth:
        CustomApplyDamage(Config.Damage, this, Evaluation.HitProjectile);
        break;
    }

    if (Config.CollisionRuleConfig.bConsumeSelfOnProjectileCollision)
    {
        bIsAlreadyExploded = true;
        SetActorEnableCollision(false);
        Destroy();
    }
}
