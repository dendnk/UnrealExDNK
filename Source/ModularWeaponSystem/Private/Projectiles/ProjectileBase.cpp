// Copyright 2024 [HeliAce: Denis Kruchok, Denis Romanovsky]. All rights reserved.


#include "Projectiles/ProjectileBase.h"

#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Impact/WeaponSystemHitSettings.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Projectiles/ProjectileCollisionRuleUtils.h"
#include "Types/WeaponTypes.h"
#include "Water/WaterHitLibrary.h"
#include "Water/WaterImpactThrottle.h"
#include "Water/WeaponSystemWaterSettings.h"

DEFINE_LOG_CATEGORY(LogProjectile);

FOnProjectileEndedNative AProjectileBase::OnProjectileEndedNative;


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

        if (IsValid(MovementComponent))
        {
            MovementComponent->OnProjectileStop.AddDynamic(this, &AProjectileBase::HandleMovementStopped);
        }
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

bool AProjectileBase::IsHomingProjectile() const
{
    return IsValid(MovementComponent) && MovementComponent->bIsHomingProjectile;
}

void AProjectileBase::LifeSpanExpired()
{
    FHitResult Hit;
    Hit.Location = GetActorLocation();
    Hit.ImpactPoint = GetActorLocation();
    SetPendingEndReason(EProjectileEndReason::LifeSpan);
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

void AProjectileBase::ExplodeProjectile(const FHitResult& Hit, bool bSuppressFx, bool bDestroyedByOtherProjectile)
{
    if (bIsAlreadyExploded)
    {
        return;
    }

    bIsAlreadyExploded = true;
    GetWorldTimerManager().ClearTimer(StuckFailsafeTimerHandle);
    SetActorEnableCollision(false);

#if !UE_BUILD_SHIPPING
    OnProjectileEndedNative.Broadcast(this, bDestroyedByOtherProjectile ? EProjectileEndReason::RuleDestroyed : PendingEndReason, Hit);
#endif

    ApplyAoEDamage(Hit);

    // Only hits whose component is the water switch effects; airbursts and interceptions keep theirs (REQ-11).
    const bool bWaterImpact = !bSuppressFx && TrySpawnWaterImpact(Hit);
    // A bullet on the player's pawn shows sparks instead of its ground effect (vfx-review-findings REQ-1).
    const bool bPlayerHit = !bSuppressFx && !bWaterImpact && ShowsPlayerHitEffect()
        && UWeaponSystemHitSettings::TrySpawnPlayerHit(this, Hit);
    if (!bSuppressFx && !bWaterImpact && !bPlayerHit && ExplosionEffect != nullptr)
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

bool AProjectileBase::TrySpawnWaterImpact(const FHitResult& Hit)
{
    if (WaterImpact == EProjectileWaterImpact::None || !UWaterHitLibrary::IsWaterHit(Hit))
    {
        return false;
    }

    const UWeaponSystemWaterSettings* Settings = UWeaponSystemWaterSettings::Get();
    const bool bBulletPuff = WaterImpact == EProjectileWaterImpact::BulletPuff;
    if (bBulletPuff)
    {
        // Per weapon: the weapon's owner fires every bullet of a burst. A throttled puff still counts as a water
        // impact, so the bullet's ground effect doesn't show on the water instead.
        UWaterImpactThrottle* Throttle = GetWorld() != nullptr ? GetWorld()->GetSubsystem<UWaterImpactThrottle>() : nullptr;
        const UObject* ThrottleKey = GetOwner() != nullptr ? static_cast<const UObject*>(GetOwner())
            : GetInstigator() != nullptr ? static_cast<const UObject*>(GetInstigator()) : static_cast<const UObject*>(GetClass());
        if (Throttle != nullptr && !Throttle->TryConsume(ThrottleKey, Settings->BulletWaterImpactMinInterval))
        {
            return true;
        }
    }

    UNiagaraSystem* Effect = WaterImpactEffect != nullptr ? WaterImpactEffect.Get()
        : (bBulletPuff ? Settings->BulletWaterEffect : Settings->ProjectileWaterEffect).LoadSynchronous();
    USoundBase* Sound = WaterImpactSound != nullptr ? WaterImpactSound.Get()
        : (bBulletPuff ? Settings->BulletWaterSound : Settings->ProjectileWaterSound).LoadSynchronous();
    const FVector Point = Hit.ImpactPoint.IsNearlyZero() ? FVector(Hit.Location) : FVector(Hit.ImpactPoint);
    const FVector SurfacePoint = UWaterHitLibrary::ProjectOntoWaterSurface(Hit.GetComponent(), Point);
    UWaterHitLibrary::SpawnWaterEffect(this, Effect, SurfacePoint);
    if (Sound != nullptr)
    {
        CustomPlaySoundAtLocation(this, Sound, SurfacePoint);
    }
    return true;
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
        SetPendingEndReason(EProjectileEndReason::Impact);
        ExplodeProjectile(Hit);
        return;

    case EProjectileCollisionRuleResult::IgnoredByRules:
        return;

    case EProjectileCollisionRuleResult::DestroyProjectile:
        if (AProjectileBase* HitProjectile = Evaluation.HitProjectile.Get())
        {
            HitProjectile->ExplodeProjectile(Hit, /*bSuppressFx=*/false, /*bDestroyedByOtherProjectile=*/true);
        }

        if (Config.CollisionRuleConfig.bConsumeSelfOnProjectileCollision)
        {
            ExplodeProjectile(Hit, /*bSuppressFx=*/false, /*bDestroyedByOtherProjectile=*/true);
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

#if !UE_BUILD_SHIPPING
    OnProjectileEndedNative.Broadcast(this, EProjectileEndReason::Disappeared, FHitResult());
#endif

    if (IdleAudioComponent != nullptr)
    {
        IdleAudioComponent->Stop();
    }

    Destroy();
}

void AProjectileBase::HandleMovementStopped(const FHitResult& ImpactResult)
{
    if (bIsAlreadyExploded)
    {
        return;
    }

    UE_LOG(LogProjectile, Log, TEXT("%s stopped without exploding after hitting %s (%s); exploding it."),
        *GetName(), *GetNameSafe(ImpactResult.GetActor()), *GetNameSafe(ImpactResult.GetComponent()));

    FHitResult Hit = ImpactResult;
    Hit.Location = GetActorLocation();
    Hit.ImpactPoint = GetActorLocation();
    SetPendingEndReason(EProjectileEndReason::MovementStopped);
    ExplodeProjectile(Hit);
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
        SetPendingEndReason(EProjectileEndReason::StuckTimer);
        ExplodeProjectile(Hit);
        return;
    }

    LastStuckCheckLocation = CurrentLocation;
}
