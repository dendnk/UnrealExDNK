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

void URocketLauncherComponent::FireProjectile()
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		UE_DNK_LOG(LogTemp, Error, "Invalid World!");
		return;
	}

	if (IsValid(WeaponDataRuntime) == false)
	{
		UE_DNK_LOG(LogTemp, Error, "Invalid WeaponData!");
		return;
	}

	if (WeaponDataRuntime->FireType != EFireType::Projectile)
	{
		UE_DNK_LOG(LogTemp, Error, "Wrong FireType [%s]!",
			*StaticEnum<EFireType>()->GetDisplayNameTextByValue(static_cast<int64>(WeaponDataRuntime->FireType)).ToString());
		return;
	}

	if (ProjectileClass == nullptr)
	{
		UE_DNK_LOG(LogTemp, Error, "Invalid ProjectileClass!");
		return;
	}

	AActor* Owner = GetOwner();
	if (IsValid(Owner) == false)
	{
		UE_DNK_LOG(LogTemp, Error, "Invalid Owner!");
		return;
	}

	ArrayUtils::CleanArray(Projectiles);

	bool bAllowToSpawnProjectile = true;
	if (GetWeaponDataRuntime()->ProjectileType == EProjectileType::HomingRocket)
	{
		if (Projectiles.Num() >= MaxActiveHomingProjectiles)
		{
			bAllowToSpawnProjectile = false;
		}
	}

	if (bAllowToSpawnProjectile == false)
	{
		UE_DNK_LOG(LogTemp, Warning, "Reach the max active homing projectiles. Failed to spawn!");
		return;
	}

	FTransform MuzzleTransform = GetMuzzleTransform();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (int32 i = 0; i < WeaponDataRuntime->AmmoPerShot; ++i)
	{
		AProjectileBase* Projectile = World->SpawnActor<AProjectileBase>(
			ProjectileClass,
			MuzzleTransform.GetLocation(),
			MuzzleTransform.GetRotation().Rotator()
		);

		SetupSpawnedProjectile(Projectile);

		Projectiles.Add(Projectile);
	}

	if (WeaponDataRuntime->bInfiniteAmmo == false)
	{
		SetCurrentAmmo(GetCurrentAmmo() - WeaponDataRuntime->AmmoPerShot);
	}

	SpawnFXAtLocation(WeaponDataRuntime->FXData.MuzzleFlashFX, MuzzleTransform.GetLocation());
	PlaySoundAtLocation(WeaponDataRuntime->FXData.FireSound, MuzzleTransform.GetLocation());
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
