// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#include "Components/WeaponComponentBase.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Projectiles/ProjectileBase.h"
#include "UI/WeaponComponentBaseWidget.h"
#include "UI/WeaponViewModel.h"
#include "UnrealExDNKUtils.h"



void UWeaponComponentBase::BeginPlay()
{
	Super::BeginPlay();

	InitWeaponData();

	WeaponViewModel = NewObject<UWeaponViewModel>(this);
	WeaponViewModel->InitializeFromWeapon(this);

	if (bShouldHaveUIWidget == false)
	{
		return;
	}

	if (UWeaponComponentBaseWidget* WeaponWidget = Cast<UWeaponComponentBaseWidget>(WidgetInstance))
	{
		WeaponWidget->SetViewModel(WeaponViewModel);
		return;
	}
	else if (IsValid(WidgetInstance))
	{
		TArray<UWidget*> AllWidgets;
		WidgetInstance->WidgetTree->GetAllWidgets(AllWidgets);
		for (UWidget* Widget : AllWidgets)
		{
			if (UWeaponComponentBaseWidget* WeaponChildWidget = Cast<UWeaponComponentBaseWidget>(Widget))
			{
				WeaponChildWidget->SetViewModel(WeaponViewModel);
				return;	//	assuming we have only one UWeaponComponentBaseWidget as a child
			}
		}
	}

	UE_DNK_LOG(LogTemp, Error, "The UI widget is not a UWeaponComponentBaseWidget or does not contain a UWeaponComponentBaseWidget!");
}

void UWeaponComponentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	WeaponViewModel->Deinit();
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
	SetCurrentAmmo(WeaponDataRuntime->MaxAmmo);
	HandleOnWeaponDataPropertyChanged();
	WeaponDataRuntime->OnWeaponDataPropertyChanged.AddDynamic(this, &ThisClass::HandleOnWeaponDataPropertyChanged);
}

void UWeaponComponentBase::StartFire_Implementation()
{
	if (bCanFire == false)
	{
		UE_DNK_LOG(LogTemp, Warning, "bCanFire is false");
		return;
	}

	if (IsValid(WeaponDataRuntime) == false)
	{
		UE_DNK_LOG(LogTemp, Error, "WeaponData is null");
		return;
	}

	if (GetCurrentAmmo() <= 0 &&
		WeaponDataRuntime->bInfiniteAmmo == false)
	{
		UE_DNK_LOG(LogTemp, Warning, "CurrentAmmo == 0!");
		return;
	}

	switch (WeaponDataRuntime->FiringMode)
	{
	case EFiringMode::SemiAuto:
		Fire();
		break;

	case EFiringMode::FullAuto:
		Fire();
		GetWorld()->GetTimerManager().SetTimer(FireLoopHandle, this, &ThisClass::Fire, WeaponDataRuntime->CooldownTime, true);
		break;

	case EFiringMode::Burst:
		Fire();
		CurrentBurstCount = 1;
		GetWorld()->GetTimerManager().SetTimer(BurstHandle, this, &ThisClass::HandleBurstFire, WeaponDataRuntime->CooldownTime, true);
		break;
	}
}

void UWeaponComponentBase::StopFire_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(FireLoopHandle);
	GetWorld()->GetTimerManager().ClearTimer(BurstHandle);
}

void UWeaponComponentBase::Fire()
{
	if (bCanFire == false)
	{
		UE_DNK_LOG(LogTemp, Warning, "bCanFire is false");
		return;
	}

	if (GetCurrentAmmo() <= 0 &&
		WeaponDataRuntime->bInfiniteAmmo == false)
	{
		UE_DNK_LOG(LogTemp, Warning, "CurrentAmmo == 0!");
		StopFire();
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

void UWeaponComponentBase::HandleBurstFire()
{
	if (++CurrentBurstCount <= WeaponDataRuntime->BurstCount)
	{
		Fire();
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(BurstHandle);
	}
}

void UWeaponComponentBase::FireProjectile()
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
		UE_LOG(LogTemp, Error, TEXT("Invalid Owner!"));
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
	}

	if (WeaponDataRuntime->bInfiniteAmmo == false)
	{
		SetCurrentAmmo(GetCurrentAmmo() - WeaponDataRuntime->AmmoPerShot);
	}

	SpawnFXAtLocation(WeaponDataRuntime->FXData.MuzzleFlashFX, MuzzleTransform.GetLocation());
	PlaySoundAtLocation(WeaponDataRuntime->FXData.FireSound, MuzzleTransform.GetLocation());
}

void UWeaponComponentBase::FireHitscan()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FTransform MuzzleTransform = GetMuzzleTransform();
	FVector Start = MuzzleTransform.GetLocation();
	FVector ShotDirection = MuzzleTransform.GetRotation().Vector();
	FVector End = Start + (ShotDirection * WeaponDataRuntime->HitscanRange);

	// Apply spread
	float SpreadAngleRad = FMath::DegreesToRadians(WeaponDataRuntime->HitscanSpread);
	ShotDirection = FMath::VRandCone(ShotDirection, SpreadAngleRad);
	End = Start + (ShotDirection * WeaponDataRuntime->HitscanRange);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	SpawnFXAtLocation(WeaponDataRuntime->FXData.MuzzleFlashFX, MuzzleTransform.GetLocation());
	PlaySoundAtLocation(WeaponDataRuntime->FXData.FireSound, MuzzleTransform.GetLocation());

	if (WeaponDataRuntime->bInfiniteAmmo == false)
	{
		SetCurrentAmmo(GetCurrentAmmo() - WeaponDataRuntime->AmmoPerShot);
	}

	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		// Deal damage
		if (AActor* HitActor = Hit.GetActor())
		{
			ApplyDamage(
				HitActor,
				ShotDirection,
				Hit,
				GetOwner()->GetInstigatorController(),
				GetOwner(),
				nullptr
				);

			SpawnFXAtLocation(WeaponDataRuntime->FXData.ImpactFX, Hit.Location, Hit.ImpactNormal.Rotation().GetInverse());
		}
	}
}

void UWeaponComponentBase::FireBeam()
{
	if (WeaponDataRuntime->FireType != EFireType::Beam)
	{
		UE_DNK_LOG(LogTemp, Error, "Wrong FireType [%s]!",
			*StaticEnum<EFireType>()->GetDisplayNameTextByValue(static_cast<int64>(WeaponDataRuntime->FireType)).ToString());
		return;
	}

	if (WeaponDataRuntime->bInfiniteAmmo == false)
	{
		SetCurrentAmmo(GetCurrentAmmo() - WeaponDataRuntime->AmmoPerShot);
	}

	// Spawn a beam FX from muzzle
	// Optionally attach a timer to apply DoT every X seconds
	if (WeaponDataRuntime->DamageData.DamagePerTick > 0)
	{
		// Start a timer to apply WeaponData->DamagePerTick over WeaponData->BeamDuration
	}
}

void UWeaponComponentBase::HandleOnWeaponDataPropertyChanged()
{
	SetProjectileClass(GetProjectileClassByType(WeaponDataRuntime->ProjectileType));
}

void UWeaponComponentBase::ApplyDamage_Implementation(AActor* DamagedActor, FVector const& HitFromDirection, FHitResult const& HitInfo, AController* EventInstigator, AActor* DamageCauser, TSubclassOf<UDamageType> DamageTypeClass)
{
	if (WeaponDataRuntime->FireType == EFireType::Hitscan)
	{
		UGameplayStatics::ApplyPointDamage(
			DamagedActor,
			WeaponDataRuntime->DamageData.BaseDamage,
			HitFromDirection,
			HitInfo,
			EventInstigator,
			DamageCauser,
			DamageTypeClass
		);
	}
}

void UWeaponComponentBase::SetCurrentAmmo(int32 NewCurrentAmmo)
{
	if (CurrentAmmo != NewCurrentAmmo)
	{
		CurrentAmmo = NewCurrentAmmo;
		if (WeaponViewModel)
		{
			WeaponViewModel->SetCurrentAmmo(CurrentAmmo);
		}
	}
}

TSubclassOf<AProjectileBase> UWeaponComponentBase::GetProjectileClassByType(const EProjectileType& ProjectileType) const
{
	if (ProjectileClasses.Contains(ProjectileType))
	{
		return *ProjectileClasses.Find(ProjectileType);
	}

	return AProjectileBase::StaticClass();
}

void UWeaponComponentBase::SetProjectileClass(TSubclassOf<AProjectileBase> NewProjectileClass)
{
	if (ProjectileClass == NewProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProjectileClass is the same!"));
		return;
	}

	ProjectileClass = NewProjectileClass;
	OnProjectileClassChanged.Broadcast(NewProjectileClass);
}

UNiagaraComponent* UWeaponComponentBase::SpawnFXAtLocation_Implementation(UNiagaraSystem* SystemTemplate, FVector Location, FRotator Rotation, FVector Scale, bool bAutoDestroy, bool bShouldAutoActivate)
{
	AActor* OwnerActor = GetOwner();
	return UNiagaraFunctionLibrary::SpawnSystemAtLocation(OwnerActor, SystemTemplate, Location, Rotation, Scale, bAutoDestroy, bShouldAutoActivate);
}

void UWeaponComponentBase::PlaySoundAtLocation_Implementation(USoundBase* Sound, FVector Location, float VolumeMultiplier, float PitchMultiplier, float StartTime)
{
	AActor* OwnerActor = GetOwner();
	UGameplayStatics::PlaySoundAtLocation(OwnerActor, Sound, Location, VolumeMultiplier, PitchMultiplier, StartTime);
}

AActor* UWeaponComponentBase::GetNearestTarget_Implementation()
{
	return nullptr;
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
		SpawnedProjectile->OnProjectileSetupFinished.Broadcast();
	}
}

FTransform UWeaponComponentBase::GetMuzzleTransform_Implementation() const
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
