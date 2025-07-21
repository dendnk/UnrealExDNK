// Copyright 2024 [HeliAce: Denis Kruchok, Denis Romanovsky]. All rights reserved.


#include "Projectiles/ProjectileBase.h"
#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
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

    //if (IsValid(MovementComponent))
    //{
        //ProjectileMovement->InitialSpeed = RocketConfig.InitialSpeed;
        //ProjectileMovement->MaxSpeed = RocketConfig.MaxSpeed;
        //ProjectileMovement->bRotationFollowsVelocity = RocketConfig.bRotationFollowsVelocity;
        //ProjectileMovement->bRotationRemainsVertical = RocketConfig.bRotationRemainsVertical;
        //ProjectileMovement->bShouldBounce = RocketConfig.bShouldBounce;
    //}
}

void AProjectileBase::BeginPlay()
{
    Super::BeginPlay();

    //IdleAudioComponent = UHeliAceFuntionsLibrary::SpawnSoundAttached(IdleSound, RocketMesh, NAME_None, FVector(ForceInit), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true, 1.f, 1.f, 0.f, nullptr, nullptr, false);

    if (ShootEffect != nullptr)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ShootEffect, GetActorLocation());
    }
}

void AProjectileBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AProjectileBase::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (IsValid(OtherActor) == false ||
        OtherActor == this ||
        OtherActor == GetOwner())
    {
        return;
    }

    //UE_LOG(LogTemp, Warning, TEXT(
    //    "=== AProjectileBase::OnRocketHit\n"
    //    "=== this : [%s]\n"
    //    "=== GetOwner() : [%s]\n"
    //    "=== HitComponent: [%s]\n"
    //    "=== OtherActor: [%s]\n"
    //    "=== OtherComp: [%s]\n"),
    //    *GetNameSafe(this),
    //    *GetNameSafe(GetOwner()),
    //    *GetNameSafe(HitComponent),
    //    *GetNameSafe(OtherActor),
    //    *GetNameSafe(OtherComp)
    //    );

    //UHeliAceFuntionsLibrary::ApplyDamage(RocketConfig.Damage, this, OtherActor);

    if (ExplosionEffect != nullptr)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(OtherActor, ExplosionEffect, Hit.Location);
    }

    if (IdleAudioComponent != nullptr)
    {
        IdleAudioComponent->Stop();
    }

    if (ExplosionSound != nullptr)
    {
        //UHeliAceFuntionsLibrary::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
    }

    Destroy();
}
