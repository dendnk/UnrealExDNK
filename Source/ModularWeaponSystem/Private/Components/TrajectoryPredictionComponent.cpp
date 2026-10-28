// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "Components/TrajectoryPredictionComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/WeaponComponentBase.h"
#include "Components/RocketLauncherComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interfaces/ILaunchVelocityProvider.h"
#include "Interfaces/IWeaponUserInterface.h"
#include "Projectiles/ProjectileBase.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealExDNKUtils.h"

DEFINE_LOG_CATEGORY(LogTrajectoryPrediction);

namespace
{
    void ReportTrajectoryPredictionFailure(uint64 MessageKey, const FString& Message)
    {
        UE_LOG(LogTrajectoryPrediction, Error, TEXT("%s"), *Message);
#if !UE_BUILD_SHIPPING
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(static_cast<int32>(MessageKey), 5.0f, FColor::Red, FString::Printf(TEXT("[TrajectoryPrediction] %s"), *Message));
        }
#endif
    }
}

UTrajectoryPredictionComponent::UTrajectoryPredictionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UTrajectoryPredictionComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UTrajectoryPredictionComponent::EnsureInitialized()
{
    bInitializationAttempted = true;

    AActor* Owner = GetOwner();
    if (IsValid(Owner) == false ||
        IsValid(GetWorld()) == false)
    {
        return;
    }

    TArray<UWeaponComponentBase*> WeaponWithProjectiles =
        ActorComponents::GetComponentsByCondition<UWeaponComponentBase>(Owner,
            [](const UWeaponComponentBase* WeaponComponent)
            {
                if (IsValid(WeaponComponent) && IsValid(WeaponComponent->GetWeaponDataRuntime()))
                {
                    return WeaponComponent->GetWeaponDataRuntime()->FireType == EFireType::Projectile;
                }

                return false;
            });

    // An owner can carry several projectile weapons (e.g. rockets + flares); previewing whichever
    // happens to be first draws the wrong weapon's arc. Prefer a weapon that reports its own real
    // launch velocity, then a rocket launcher, and only then fall back to the first one found.
    Weapon = nullptr;
    int32 BestPriority = -1;
    for (UWeaponComponentBase* Candidate : WeaponWithProjectiles)
    {
        const int32 Priority = Cast<ILaunchVelocityProvider>(Candidate) ? 2 : (Cast<URocketLauncherComponent>(Candidate) ? 1 : 0);
        if (Priority > BestPriority)
        {
            BestPriority = Priority;
            Weapon = Candidate;
        }
    }
    if (Weapon.IsValid() == false)
    {
        ReportTrajectoryPredictionFailure(static_cast<uint64>(GetUniqueID()), FString::Printf(TEXT("No projectile weapon found on '%s'; trajectory prediction disabled."), *Owner->GetName()));
        SetComponentTickEnabled(false);
        return;
    }

    WeaponParentComponent = IWeaponUserInterface::Execute_GetParentAttachment(Weapon->GetOwner());

    DotInstances = NewObject<UInstancedStaticMeshComponent>(Owner, UInstancedStaticMeshComponent::StaticClass(), DotInstancesName);
    if (DotInstances)
    {
        DotInstances->SetStaticMesh(DotMesh);
        DotInstances->SetMaterial(0, DotMaterial);
        DotInstances->SetMobility(EComponentMobility::Movable);
        DotInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        DotInstances->RegisterComponent();
        DotInstances->AttachToComponent(WeaponParentComponent.Get(), FAttachmentTransformRules::KeepRelativeTransform);
    }
}

void UTrajectoryPredictionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    if (bInitializationAttempted == false)
    {
        EnsureInitialized();
    }

    if (Weapon.IsValid())
    {
        // Mirror URocketLauncherComponent::FireProjectile(): the real shot resolves the muzzle with
        // Shot usage (not Preview), flattens the direction onto the Y=0 lane, and spawns the rocket
        // ProjectileSpawnForwardOffset ahead of the muzzle. Without this the dots diverge from the flight.
        FTransform MuzzleTransform = Weapon->GetMuzzleTransform();
        if (AActor* WeaponOwner = Weapon->GetOwner();
            IsValid(WeaponOwner) && WeaponOwner->GetClass()->ImplementsInterface(UWeaponUserInterface::StaticClass()))
        {
            const FTransform ShotTransform = IWeaponUserInterface::Execute_GetMuzzleTransform(
                WeaponOwner, Weapon.Get(), Weapon->GetWeaponDataRuntime()->MuzzleSocketName, EWeaponMuzzleTransformUsage::Shot);
            if (!ShotTransform.Equals(FTransform::Identity))
            {
                MuzzleTransform = ShotTransform;
            }
        }

        FVector ProjectileForwardDirection = MuzzleTransform.GetRotation().Vector();
        FVector StartLocation = MuzzleTransform.GetLocation();
        if (Cast<URocketLauncherComponent>(Weapon.Get()))
        {
            ProjectileForwardDirection.Y = 0.0f;
            ProjectileForwardDirection = ProjectileForwardDirection.GetSafeNormal();
            StartLocation += ProjectileForwardDirection * URocketLauncherComponent::ProjectileSpawnForwardOffset;
            StartLocation.Y = 0.0f;
        }
        // Use the runtime copy: SetupSpawnedProjectile() launches rockets with WeaponDataRuntime's
        // ProjectileSpeed, so runtime changes (upgrades, manual tweaks) must show up in the preview too.
        const UWeaponDataAsset* SpeedSource = Weapon->GetWeaponDataRuntime();
        if (IsValid(SpeedSource) == false)
        {
            SpeedSource = Weapon->GetWeaponDataAsset();
        }

        if (IsValid(SpeedSource))
        {
            // Weapons that know their real launch velocity (e.g. the player rocket launcher adds the
            // helicopter's velocity) provide it so the dots match the flight; everything else keeps the
            // plain speed * direction legacy behavior.
            const ILaunchVelocityProvider* VelocityProvider = Cast<ILaunchVelocityProvider>(Weapon.Get());
            const FVector ProjectileInitialVelocity = VelocityProvider
                ? VelocityProvider->GetPredictedLaunchVelocity(ProjectileForwardDirection)
                : SpeedSource->ProjectileSpeed * ProjectileForwardDirection;
            PredictAndDrawTrajectory(StartLocation, ProjectileInitialVelocity);
        }
    }

    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTrajectoryPredictionComponent::PredictAndDrawTrajectory(const FVector& StartLocation, const FVector& LaunchVelocity)
{
    if (IsValid(GetOwner()) == false ||
        IsValid(GetWorld()) == false ||
        Weapon.IsValid() == false ||
        WeaponParentComponent.IsValid() == false ||
        DotInstances == nullptr)
    {
        ReportTrajectoryPredictionFailure(static_cast<uint64>(GetUniqueID()) + 1, TEXT("Trajectory prediction cannot render: owner, world, weapon, parent attachment, or dot instances invalid."));
        return;
    }

    FPredictProjectilePathParams Params;
    Params.StartLocation = StartLocation;
    Params.LaunchVelocity = LaunchVelocity;
    Params.ProjectileRadius = ProjectileRadius;
    Params.MaxSimTime = MaxSimTime;
    Params.SimFrequency = MaxSteps;
    Params.TraceChannel = ECC_Visibility;
    Params.ActorsToIgnore.Add(GetOwner());

    FPredictProjectilePathResult Result;
    float StraightTime = 0.f;
    float FallGravityScale = 0.f;
    float LifeSpan = 0.f;
    const ILaunchVelocityProvider* ProfileProvider = Cast<ILaunchVelocityProvider>(Weapon.Get());
    if (ProfileProvider && ProfileProvider->GetPredictedFlightProfile(StraightTime, FallGravityScale, LifeSpan))
    {
        // The weapon switches its rockets' gravity mid-flight: predict the same two phases, all the way to the first
        // blocking hit (ground, enemy, cover), so the dots end where the rocket lands.
        const float FallGravityZ = bHasGravity ? GetWorld()->GetGravityZ() * FallGravityScale : 0.f;
        Params.bTraceWithCollision = true;
        PredictTwoPhasePath(this, Params, StraightTime, FallGravityZ, LifeSpan, FallSimFrequency, Result);
    }
    else
    {
        // The rocket's real gravity comes from its movement component's ProjectileGravityScale, so read
        // it from the projectile class defaults; otherwise the preview drops faster than a
        // reduced-gravity rocket actually flies.
        float GravityScale = 1.f;
        if (const TSubclassOf<AProjectileBase> ProjectileClass = Weapon->GetProjectileClass())
        {
            if (const AProjectileBase* ProjectileCDO = ProjectileClass->GetDefaultObject<AProjectileBase>())
            {
                if (const UProjectileMovementComponent* Movement = ProjectileCDO->FindComponentByClass<UProjectileMovementComponent>())
                {
                    GravityScale = Movement->ProjectileGravityScale;
                }
            }
        }
        Params.OverrideGravityZ = bHasGravity ? GetWorld()->GetGravityZ() * GravityScale : 0;
        UGameplayStatics::PredictProjectilePath(this, Params, Result);
    }
    LastPath = Result;
    LastStartLocation = StartLocation;
    LastLaunchVelocity = LaunchVelocity;
    LastPathFrame = GFrameCounter;
    {
        DotInstances->ClearInstances();

        const APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
        const bool bHasCamera = IsValid(CameraManager);
        const FVector FallbackFacing = -GetOwner()->GetActorForwardVector();
        const float Scale = DotSize / 100.f;

        // A long path draws every DotStride-th sample, always ending on the last one (the hit point).
        const int32 NumPoints = Result.PathData.Num();
        const int32 DotStride = (MaxDots > 0 && NumPoints > MaxDots) ? FMath::DivideAndRoundUp(NumPoints, MaxDots) : 1;
        int32 PreviousDrawn = INDEX_NONE;

        for (int32 i = 0; i < NumPoints; ++i)
        {
            if (i % DotStride != 0 && i != NumPoints - 1)
            {
                continue;
            }

            const FVector& DotLocation = Result.PathData[i].Location;
            FVector FacingDirection = FallbackFacing;
            if (bHasCamera)
            {
                const FVector ToCamera = CameraManager->GetCameraLocation() - DotLocation;
                if (ToCamera.SizeSquared() > UE_KINDA_SMALL_NUMBER)
                {
                    FacingDirection = ToCamera.GetUnsafeNormal();
                }
            }

            const FRotator DotRotation = FRotationMatrix::MakeFromZ(FacingDirection).Rotator();
            const FTransform DotTransform(DotRotation, DotLocation, FVector(Scale));
            const bool bWorldSpace = true;
            DotInstances->AddInstance(DotTransform, bWorldSpace);

            if (bDrawDebug && PreviousDrawn != INDEX_NONE)
            {
                DrawDebugLine(
                    GetWorld(),
                    Result.PathData[PreviousDrawn].Location,
                    Result.PathData[i].Location,
                    DebugColor,
                    bDebugPersistentLines,
                    DebugLifeTime,
                    DebugDepthPriority,
                    DebugThickness
                );
            }
            PreviousDrawn = i;
        }
    }
}

void UTrajectoryPredictionComponent::PredictTwoPhasePath(const UObject* WorldContextObject, const FPredictProjectilePathParams& Params,
    float StraightTime, float FallGravityZ, float LifeSpan, float InFallSimFrequency, FPredictProjectilePathResult& OutResult)
{
    OutResult.Reset();
    const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (World == nullptr)
    {
        return;
    }

    const FVector Velocity = Params.LaunchVelocity;
    const float FallEndTime = LifeSpan > 0.f ? LifeSpan : FMath::Max(StraightTime, 0.f) + Params.MaxSimTime;
    StraightTime = FMath::Clamp(StraightTime, 0.f, FallEndTime);

    // Straight part: velocity is constant, so one sweep finds its hit; the samples only space the dots.
    const FVector StraightEnd = Params.StartLocation + Velocity * StraightTime;
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PredictTwoPhasePath), Params.bTraceComplex);
    QueryParams.AddIgnoredActors(Params.ActorsToIgnore);
    FHitResult StraightHit;
    bool bStraightHit = false;
    if (StraightTime > 0.f && Params.bTraceWithCollision && Params.bTraceWithChannel)
    {
        bStraightHit = Params.ProjectileRadius > 0.f
            ? World->SweepSingleByChannel(StraightHit, Params.StartLocation, StraightEnd, FQuat::Identity, Params.TraceChannel,
                FCollisionShape::MakeSphere(Params.ProjectileRadius), QueryParams)
            : World->LineTraceSingleByChannel(StraightHit, Params.StartLocation, StraightEnd, Params.TraceChannel, QueryParams);
    }
    const float StraightEndTime = bStraightHit ? StraightTime * StraightHit.Time : StraightTime;

    const float SampleStep = 1.f / FMath::Max(Params.SimFrequency, 1.f);
    for (float Time = 0.f; Time < StraightEndTime; Time += SampleStep)
    {
        OutResult.PathData.Emplace(Params.StartLocation + Velocity * Time, Velocity, Time);
    }
    OutResult.PathData.Emplace(Params.StartLocation + Velocity * StraightEndTime, Velocity, StraightEndTime);
    OutResult.LastTraceDestination = OutResult.PathData.Last();

    if (bStraightHit)
    {
        OutResult.HitResult = StraightHit;
        return;
    }

    const float FallTime = FallEndTime - StraightTime;
    if (FallTime <= UE_KINDA_SMALL_NUMBER)
    {
        return;
    }

    // Fall part: the stock predictor from the straight part's end, under the stronger gravity, to the first hit.
    // OverrideGravityZ 0 means "world gravity" to PredictProjectilePath, so a gravity-free fall uses a tiny value.
    FPredictProjectilePathParams FallParams = Params;
    FallParams.StartLocation = StraightEnd;
    FallParams.MaxSimTime = FallTime;
    FallParams.SimFrequency = FMath::Max(InFallSimFrequency, 1.f);
    FallParams.OverrideGravityZ = FallGravityZ != 0.f ? FallGravityZ : -UE_KINDA_SMALL_NUMBER;
    FPredictProjectilePathResult FallResult;
    UGameplayStatics::PredictProjectilePath(WorldContextObject, FallParams, FallResult);

    // Its first point is the straight part's end, already in the path.
    for (int32 Index = 1; Index < FallResult.PathData.Num(); ++Index)
    {
        FPredictProjectilePathPointData Point = FallResult.PathData[Index];
        Point.Time += StraightTime;
        OutResult.PathData.Add(Point);
    }
    OutResult.HitResult = FallResult.HitResult;
    OutResult.LastTraceDestination = FallResult.LastTraceDestination;
    OutResult.LastTraceDestination.Time += StraightTime;
}
