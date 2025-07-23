// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#include "Components/WeaponComponentBase.h"
#include <Kismet/GameplayStatics.h>
#include "Projectiles/ProjectileBase.h"
#include "UI/WeaponComponentBaseWidget.h"
#include "UI/WeaponViewModel.h"
#include "UnrealExDNKUtils.h"
#include "View/MVVMView.h"



void UWeaponComponentBase::BeginPlay()
{
	Super::BeginPlay();

	InitWeaponData();

	WeaponViewModel = NewObject<UWeaponViewModel>(this);
	WeaponViewModel->InitializeFromWeaponDataAsset(WeaponDataRuntime);

	if (bShouldHaveUIWidget == false)
	{
		return;
	}

	if (UWeaponComponentBaseWidget* WeaponWidget = Cast<UWeaponComponentBaseWidget>(WidgetInstance))
	{
		WeaponWidget->SetViewModel(WeaponViewModel);
		return;
	}

	UE_DNK_LOG(LogTemp, Error, "UI widget is not UWeaponComponentBaseWidget!");
}

void UWeaponComponentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	WeaponViewModel = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UWeaponComponentBase::InitWeaponData()
{
	if (IsValid(WeaponDataAsset) == false)
	{
		UE_DNK_LOG(LogTemp, Error, "WeaponDataAsset is invalid!");
		return;
	}

	WeaponDataRuntime = DuplicateObject<UWeaponDataAsset>(GetWeaponDataAsset(), this);
	CurrentAmmo = WeaponDataRuntime->MaxAmmo;
}

void UWeaponComponentBase::Fire()
{
	if (IsValid(WeaponDataRuntime) == false)
	{
		UE_DNK_LOG(LogTemp, Error, "WeaponData is null");
		return;
	}

	if (CurrentAmmo == 0)
	{
		UE_DNK_LOG(LogTemp, Warning, "CurrentAmmo == 0!");
		return;
	}

	switch (WeaponDataRuntime->FireType)
	{
	case EFireType::Projectile:
		FireProjectile();
		break;

	case EFireType::Hitscan:
		FireHitscan();
		break;

	case EFireType::Beam:
		FireBeam();
		break;

	default:
		UE_DNK_LOG(LogTemp, Warning, "Unknown FireType in WeaponComponent");
		break;
	}
}

void UWeaponComponentBase::FireProjectile()
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		UE_DNK_LOG(LogTemp, Warning, "Invalid World!");
		return;
	}

	if (IsValid(WeaponDataRuntime) == false)
	{
		UE_DNK_LOG(LogTemp, Warning, "Invalid WeaponData!");
		return;
	}

	if (IsValid(WeaponDataRuntime->ProjectileClass) == false)
	{
		UE_DNK_LOG(LogTemp, Warning, "Invalid ProjectileClass!");
		return;
	}

	AActor* Owner = GetOwner();
	if (IsValid(Owner) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Owner!"));
		return;
	}

	FTransform MuzzleTransform = BP_GetMuzzleTransform();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (int32 i = 0; i < WeaponDataRuntime->ProjectilesPerShot; ++i)
	{
		AProjectileBase* Projectile = World->SpawnActor<AProjectileBase>(
			WeaponDataRuntime->ProjectileClass,
			MuzzleTransform.GetLocation(),
			MuzzleTransform.GetRotation().Rotator()
		);

		SetupSpawnedProjectile(Projectile);
	}
}

void UWeaponComponentBase::FireHitscan()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FTransform MuzzleTransform = BP_GetMuzzleTransform();
	FVector Start = MuzzleTransform.GetLocation();
	FVector ShotDirection = MuzzleTransform.GetRotation().Vector();
	FVector End = Start + (ShotDirection * WeaponDataRuntime->HitscanRange);

	// Apply spread
	float SpreadAngleRad = FMath::DegreesToRadians(WeaponDataRuntime->Spread);
	ShotDirection = FMath::VRandCone(ShotDirection, SpreadAngleRad);
	End = Start + (ShotDirection * WeaponDataRuntime->HitscanRange);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		// Deal damage
		if (AActor* HitActor = Hit.GetActor())
		{
			UGameplayStatics::ApplyPointDamage(
				HitActor,
				WeaponDataRuntime->DamageData.BaseDamage,
				ShotDirection,
				Hit,
				GetOwner()->GetInstigatorController(),
				GetOwner(),
				nullptr
			);
		}
	}

	// Optional: FX
}

void UWeaponComponentBase::FireBeam()
{
	if (!WeaponDataRuntime->bIsBeam)
		return;

	// Spawn a beam FX from muzzle
	// Optionally attach a timer to apply DoT every X seconds
	if (WeaponDataRuntime->bApplyDoT)
	{
		// Start a timer to apply WeaponData->DamagePerTick over WeaponData->BeamDuration
	}
}

void UWeaponComponentBase::BP_Fire_Implementation()
{
	if (CanFire())
	{
		Fire();
	}
}

bool UWeaponComponentBase::CanFire() const
{
	return true;
}

void UWeaponComponentBase::SetupSpawnedProjectile(AProjectileBase* SpawnedProjectile)
{
	AActor* Owner = GetOwner();
	if (IsValid(Owner) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Owner!"));
		return;
	}

	if (IsValid(SpawnedProjectile))
	{
		SpawnedProjectile->SetInstigator(Owner->GetInstigator());
		SpawnedProjectile->MeshComponent->IgnoreActorWhenMoving(GetOwner(), true);
	}
}

FTransform UWeaponComponentBase::BP_GetMuzzleTransform_Implementation()
{
	if (IsValid(WeaponDataRuntime) == false)
	{
		UE_DNK_LOG(LogTemp, Error, "Invalid WeaponData!");
		return FTransform::Identity;
	}

	AActor* Owner = GetOwner();
	if (IsValid(Owner) == false)
	{
		UE_DNK_LOG(LogTemp, Error, "Invalid Owner!");
		return FTransform::Identity;
	}

	if (USkeletalMeshComponent* SkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
	{
		return SkeletalMesh->GetSocketTransform(WeaponDataRuntime->MuzzleSocketName);
	}
	else if (UStaticMeshComponent* StaticMesh = Owner->FindComponentByClass<UStaticMeshComponent>())
	{
		return StaticMesh->GetSocketTransform(WeaponDataRuntime->MuzzleSocketName);
	}
	else if (USceneComponent* SceneComponent = Owner->FindComponentByClass<USceneComponent>())
	{
		return SceneComponent->GetSocketTransform(WeaponDataRuntime->MuzzleSocketName);
	}

	UE_DNK_LOG(LogTemp, Error, "Setup MuzzleSocketName for you Weapon Component!");

	return FTransform::Identity;
}
