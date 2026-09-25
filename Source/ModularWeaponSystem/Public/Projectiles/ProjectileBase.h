// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "Types/WeaponTypes.h"
#include "ProjectileBase.generated.h"

class AProjectileBase;
class UAudioComponent;
class UInitialActiveSoundParams;
class UNiagaraSystem;
class UProjectileMovementComponent;
class USoundBase;
class UStaticMeshComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogProjectile, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProjectileSetupFinishedDelegate);

// Why a projectile ended. Reported only in non-shipping builds: lets tests (e.g. the HeliAce
// collision matrix) tell a rule air-burst from an impact, a stop, a timeout or a silent removal.
enum class EProjectileEndReason : uint8
{
    Other,
    Impact,
    RuleDestroyed,
    MovementStopped,
    StuckTimer,
    LifeSpan,
    HealthExhausted,
    Disappeared,
};

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnProjectileEndedNative, AProjectileBase* /*Projectile*/, EProjectileEndReason /*Reason*/, const FHitResult& /*Hit*/);

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
    virtual void LifeSpanExpired() override;

    UFUNCTION(BlueprintCallable, Category = Projectiles)
    virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    // bDestroyedByOtherProjectile: true when this explosion was caused by another projectile
    // destroying this one in flight (projectile-vs-projectile collision rules), as opposed to a
    // ground/target hit, lifespan expiry, or the stuck failsafe. Lets a subclass (e.g. a rocket
    // that scatters hazards on landing) tell an air-burst apart from a normal landing.
    UFUNCTION(BlueprintCallable, Category="Projectile")
    virtual void ExplodeProjectile(const FHitResult& Hit, bool bSuppressFx = false, bool bDestroyedByOtherProjectile = false);

    // Silently removes the projectile with no explosion FX/sound/AoE damage. Used when a hit
    // shouldn't cause a reaction at all (e.g. the stuck failsafe below).
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    virtual void DisappearProjectile();

    // Gives the projectile a target actor to react to (e.g. steer towards). No-op unless a
    // subclass overrides it; the weapon that spawned this projectile calls it with its
    // nearest valid target right after spawn.
    UFUNCTION(BlueprintCallable, Category = Projectiles)
    virtual void SetProjectileTarget(AActor* NewTarget) {}

    // True when this projectile steers toward a target (built-in movement homing or a subclass's own
    // steering). Weapons use it to apply homing-only tuning such as the launch speed multiplier.
    virtual bool IsHomingProjectile() const;

protected:
    virtual void BeginPlay() override;

    // Periodic check bound to a timer (not Tick, so non-ticking projectiles stay non-ticking):
    // if the projectile hasn't moved meaningfully since the last check, something stopped it
    // without exploding it (e.g. a collision rule result that intentionally does nothing), so
    // it explodes instead of sitting frozen in place forever.
    void CheckForStuckProjectile();

    // Immediate counterpart of the check above: the movement component stops the projectile on
    // any blocking hit it doesn't bounce off, even when the hit handling chose not to explode
    // (ignored collision rule, hit on the owner, invalid hit actor). Exploding here means the
    // projectile never visibly hangs in the air. Shares the StuckFailsafeSeconds opt-out.
    UFUNCTION()
    void HandleMovementStopped(const FHitResult& ImpactResult);
    virtual UAudioComponent* CustomSpawnSoundAttached(USoundBase* Sound, USceneComponent* AttachToComponent, FName AttachPointName = NAME_None, FVector Location = FVector(ForceInit), FRotator Rotation = FRotator::ZeroRotator, EAttachLocation::Type LocationType = EAttachLocation::KeepRelativeOffset, bool bStopWhenAttachedToDestroyed = false, float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f, float StartTime = 0.f, USoundAttenuation* AttenuationSettings = nullptr, USoundConcurrency* ConcurrencySettings = nullptr, bool bAutoDestroy = true);
    virtual float CustomApplyDamage(float Damage, AActor* DamageCauser, AActor* OtherActor, TSubclassOf<UDamageType> DamageTypeClass = nullptr);

    virtual void ApplyAoEDamage(const FHitResult& Hit) {}
    virtual void CustomPlaySoundAtLocation(const UObject* WorldContextObject, USoundBase* Sound, FVector Location, float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f, float StartTime = 0.f, class USoundAttenuation* AttenuationSettings = nullptr, USoundConcurrency* ConcurrencySettings = nullptr, const UInitialActiveSoundParams* InitialParams = nullptr);
    virtual void HandleProjectileCollisionHit(AActor* HitActor, const FHitResult& Hit);

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = BaseRocket)
    TObjectPtr<UProjectileMovementComponent> MovementComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = BaseRocket)
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BaseRocket)
    TObjectPtr<UNiagaraSystem> ExplosionEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BaseRocket)
    TObjectPtr<USoundBase> ExplosionSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BaseRocket)
    TObjectPtr<USoundBase> IdleSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BaseRocket)
    FProjectileConfig Config;

    UPROPERTY(BlueprintAssignable)
    FOnProjectileSetupFinishedDelegate OnProjectileSetupFinished;

    // Dev-only (no-op in shipping): broadcast once per projectile from ExplodeProjectile /
    // DisappearProjectile with the cause and the hit it exploded with (empty for a disappearance).
    static FOnProjectileEndedNative OnProjectileEndedNative;

protected:
    // Internal explode call sites set this right before ExplodeProjectile so the broadcast carries
    // the cause; anything that calls ExplodeProjectile without setting it reports Other.
    void SetPendingEndReason(EProjectileEndReason Reason) { PendingEndReason = Reason; }

private:
    EProjectileEndReason PendingEndReason = EProjectileEndReason::Other;

    UPROPERTY()
    TObjectPtr<UAudioComponent> IdleAudioComponent;

    bool bIsAlreadyExploded = false;

    FTimerHandle StuckFailsafeTimerHandle;
    FVector LastStuckCheckLocation = FVector::ZeroVector;
};
