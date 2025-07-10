// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#include "Components/RocketLauncherComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UnrealExDNKUtils.h"

URocketLauncherComponent::URocketLauncherComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URocketLauncherComponent::Fire()
{
    if (!WeaponData || WeaponData->FireType != EFireType::Projectile)
    {
        UE_DNK_LOG(LogTemp, Warning, "Invalid FireType or missing WeaponData.");
        return;
    }

    SpawnProjectile();
}

void URocketLauncherComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URocketLauncherComponent::SpawnProjectile()
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor || !WeaponData->ProjectileClass)
    {
        UE_DNK_LOG(LogTemp, Warning, "Missing owner or ProjectileClass.");
        return;
    }

    // Try to find a muzzle socket from a SkeletalMeshComponent (if any)
    FTransform MuzzleTransform;
    if (USkeletalMeshComponent* Mesh = OwnerActor->FindComponentByClass<USkeletalMeshComponent>())
    {
        if (Mesh->DoesSocketExist(WeaponData->MuzzleSocketName))
        {
            MuzzleTransform = Mesh->GetSocketTransform(WeaponData->MuzzleSocketName);
        }
        else
        {
            MuzzleTransform = Mesh->GetComponentTransform();
        }
    }
    else
    {
        MuzzleTransform = OwnerActor->GetActorTransform();
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerActor;
    SpawnParams.Instigator = OwnerActor->GetInstigator();

    AActor* Projectile = GetWorld()->SpawnActor<AActor>(
        WeaponData->ProjectileClass,
        MuzzleTransform.GetLocation(),
        MuzzleTransform.GetRotation().Rotator(),
        SpawnParams
    );

    // Apply velocity if it has ProjectileMovementComponent
    if (Projectile)
    {
        if (UProjectileMovementComponent* Movement = Projectile->FindComponentByClass<UProjectileMovementComponent>())
        {
            Movement->Velocity = MuzzleTransform.GetRotation().Vector() * WeaponData->ProjectileSpeed;
        }
    }
}
