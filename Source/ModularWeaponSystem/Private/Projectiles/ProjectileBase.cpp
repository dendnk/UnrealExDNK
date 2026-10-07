// Copyright 2024 [HeliAce: Denis Kruchok, Denis Romanovsky]. All rights reserved.


#include "Projectiles/ProjectileBase.h"

#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Projectiles/ProjectileCollisionRuleUtils.h"
#include "Types/WeaponTypes.h"

DEFINE_LOG_CATEGORY(LogProjectile);


AProjectileBase::AProjectileBase()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(WeaponSystemNames::ProjectileMesh);
    if (IsValid(MeshComponent))
    {
        RootComponent = MeshComponent;
        MeshComponent->OnComponentHit.AddDynamic(this, &AProjectileBase::OnProjectileHit);
    }

    MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(WeaponSystemNames::ProjectileMovement);
    if (IsValid(MovementComponent))
    {
        MovementComponent->UpdatedComponent = MeshComponent;
    }
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

    if (Config.StuckFailsafeSeconds > 0.f)
    {
        LastStuckCheckLocation = GetActorLocation();
        GetWorldTimerManager().SetTimer(StuckFailsafeTimerHandle, this, &AProjectileBase::CheckForStuckProjectile, Config.StuckFailsafeSeconds, true);
    }
}

UAudioComponent* AProjectileBase::CustomSpawnSoundAttached(USoundBase* Sound, USceneComponent* AttachToComponent, FName AttachPointName, FVector Location, FRotator Rotation, EAttachLocation::Type LocationType, bool bStopWhenAttachedToDestroyed, float VolumeMultiplier, float PitchMultiplier, float StartTime, USoundAttenuation* AttenuationSettings, USoundConcurrency* ConcurrencySettings, bool bAutoDestroy)
{
    return UGameplayStatics::SpawnSoundAttached(Sound, AttachToComponent, AttachPointName, Location, LocationType, bStopWhenAttachedToDestroyed, VolumeMultiplier, PitchMultiplier, StartTime, AttenuationSettings, ConcurrencySettings, bAutoDestroy);
}

float AProjectileBase::CustomApplyDamage(float Damage, AActor* DamageCauser, AActor* OtherActor, TSubclassOf<UDamageType> DamageTypeClass)
{
    AController* InstigatorController = DamageCauser != nullptr
                                        ? DamageCauser->GetInstigatorController()
                                        : nullptr;
    return UGameplayStatics::ApplyDamage(OtherActor, Damage, InstigatorController, DamageCauser, DamageTypeClass);
}

void AProjectileBase::CustomPlaySoundAtLocation(const UObject* WorldContextObject, USoundBase* Sound, FVector Location, float VolumeMultiplier, float PitchMultiplier, float StartTime, USoundAttenuation* AttenuationSettings, USoundConcurrency* ConcurrencySettings, const UInitialActiveSoundParams* InitialParams)
{
    return UGameplayStatics::PlaySoundAtLocation(WorldContextObject, Sound, Location, VolumeMultiplier, PitchMultiplier, StartTime, AttenuationSettings, ConcurrencySettings, InitialParams);
}

void AProjectileBase::LifeSpanExpired()
{
    FHitResult Hit;
    Hit.Location = GetActorLocation();
    Hit.ImpactPoint = GetActorLocation();
    ExplodeProjectile(Hit, Config.bSuppressExplosionFxOnLifespanExpiry);
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

void AProjectileBase::ExplodeProjectile(const FHitResult& Hit, bool bSuppressFx)
{
    if (bIsAlreadyExploded)
    {
        return;
    }

    bIsAlreadyExploded = true;
    GetWorldTimerManager().ClearTimer(StuckFailsafeTimerHandle);
    SetActorEnableCollision(false);

    ApplyAoEDamage(Hit);

    if (!bSuppressFx && ExplosionEffect != nullptr)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ExplosionEffect, Hit.Location);
    }

    if (IdleAudioComponent != nullptr)
    {
        IdleAudioComponent->Stop();
    }

    if (!bSuppressFx && ExplosionSound != nullptr)
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
        if (!Config.bHasAoEOnExplode)
        {
            CustomApplyDamage(Config.Damage, this, HitActor);
        }
        ExplodeProjectile(Hit);
        return;

    case EProjectileCollisionRuleResult::IgnoredByRules:
        return;

    case EProjectileCollisionRuleResult::DestroyProjectile:
        if (AProjectileBase* HitProjectile = Evaluation.HitProjectile.Get())
        {
            HitProjectile->ExplodeProjectile(Hit);
        }

        if (Config.CollisionRuleConfig.bConsumeSelfOnProjectileCollision)
        {
            ExplodeProjectile(Hit);
        }
        return;
    }
}

void AProjectileBase::DisappearProjectile()
{
    if (bIsAlreadyExploded)
    {
        return;
    }

    bIsAlreadyExploded = true;
    GetWorldTimerManager().ClearTimer(StuckFailsafeTimerHandle);
    SetActorEnableCollision(false);

    if (IdleAudioComponent != nullptr)
    {
        IdleAudioComponent->Stop();
    }

    Destroy();
}

void AProjectileBase::CheckForStuckProjectile()
{
    if (bIsAlreadyExploded)
    {
        GetWorldTimerManager().ClearTimer(StuckFailsafeTimerHandle);
        return;
    }

    constexpr float StationaryDistanceThreshold = 5.f; // cm
    const FVector CurrentLocation = GetActorLocation();

    if (FVector::DistSquared(CurrentLocation, LastStuckCheckLocation) <= FMath::Square(StationaryDistanceThreshold))
    {
        FHitResult Hit;
        Hit.Location = CurrentLocation;
        Hit.ImpactPoint = CurrentLocation;
        ExplodeProjectile(Hit);
        return;
    }

    LastStuckCheckLocation = CurrentLocation;
}
