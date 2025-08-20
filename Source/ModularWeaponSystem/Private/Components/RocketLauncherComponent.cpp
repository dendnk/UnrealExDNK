// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#include "Components/RocketLauncherComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Projectiles/ProjectileBase.h"
#include "UnrealExDNKUtils.h"

URocketLauncherComponent::URocketLauncherComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = TickInterval;

    ResetCachedRocketBounds();
}

AActor* URocketLauncherComponent::GetNearestTarget_Implementation()
{
    return Super::GetNearestTarget();
}

void URocketLauncherComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URocketLauncherComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearHomingRocketData();

    Super::EndPlay(EndPlayReason);
}

void URocketLauncherComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateHomingRocketData();
}

void URocketLauncherComponent::SetupSpawnedProjectile(AProjectileBase* SpawnedProjectile)
{
    if (SpawnedProjectile)
    {
        if (UProjectileMovementComponent* Movement = SpawnedProjectile->FindComponentByClass<UProjectileMovementComponent>())
        {
            Movement->Velocity = GetMuzzleTransform().GetRotation().Vector() * WeaponDataRuntime->ProjectileSpeed;
            if (GetWeaponDataRuntime()->ProjectileType == EProjectileType::HomingRocket)
            {
                if (AActor* Actor = GetNearestTarget())
                {
                    Movement->HomingTargetComponent = Actor->GetRootComponent();

                    HomingTargets.Add({ SpawnedProjectile, Actor });
                }
            }
        }
    }

    Super::SetupSpawnedProjectile(SpawnedProjectile);
}

void URocketLauncherComponent::ResetCachedRocketBounds()
{
    CachedRocketBounds.Origin = FVector::ZeroVector;
    CachedRocketBounds.BoxExtent = FVector::ZeroVector;
    CachedRocketBounds.SphereRadius = 0.f;
}

void URocketLauncherComponent::UpdateHomingRocketData()
{
    HomingTargetsLocation.Empty();
    for (auto It = HomingTargets.CreateIterator(); It; ++It)
    {
        if (It.Key().IsValid() == false)
        {
            It.RemoveCurrent();
            continue;
        }

        if (AProjectileBase* Projectile = It.Key().Get())
        {
            AActor* Target = It.Value();
            HomingTargetsLocation.AddUnique(Target->GetActorLocation());
        }
    }
}

void URocketLauncherComponent::ClearHomingRocketData()
{
    HomingTargets.Empty();
    HomingTargetsLocation.Empty();
}
