// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#include "Components/RocketLauncherComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Projectiles/ProjectileBase.h"
#include "UnrealExDNKUtils.h"

URocketLauncherComponent::URocketLauncherComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

    ResetCachedRocketBounds();
}

void URocketLauncherComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URocketLauncherComponent::SetupSpawnedProjectile(AProjectileBase* SpawnedProjectile)
{
    if (SpawnedProjectile)
    {
        if (UProjectileMovementComponent* Movement = SpawnedProjectile->FindComponentByClass<UProjectileMovementComponent>())
        {
            Movement->Velocity = BP_GetMuzzleTransform().GetRotation().Vector() * WeaponDataRuntime->ProjectileSpeed;
        }
    }
}

void URocketLauncherComponent::ResetCachedRocketBounds()
{
    CachedRocketBounds.Origin = FVector::ZeroVector;
    CachedRocketBounds.BoxExtent = FVector::ZeroVector;
    CachedRocketBounds.SphereRadius = 0.f;
}
