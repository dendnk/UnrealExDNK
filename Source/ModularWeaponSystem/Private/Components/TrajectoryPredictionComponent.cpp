// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "Components/TrajectoryPredictionComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/WeaponComponentBase.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Camera/PlayerCameraManager.h"
#include "Interfaces/IWeaponUserInterface.h"
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
                if (IsValid(WeaponComponent))
                {
                    return WeaponComponent->GetWeaponDataRuntime()->FireType == EFireType::Projectile;
                }

                return false;
            });

    Weapon = WeaponWithProjectiles.Num() > 0 ? WeaponWithProjectiles[0] : nullptr;
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

    Super::BeginPlay();
}

void UTrajectoryPredictionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    if (Weapon.IsValid())
    {
        const FVector StartLocation = Weapon->GetMuzzleTransform().GetLocation();
        const FVector ProjectileForwardDirection = Weapon->GetMuzzleTransform().GetRotation().GetForwardVector();
        const FVector ProjectileInitialVelocity = Weapon->GetWeaponDataAsset()->ProjectileSpeed * ProjectileForwardDirection;

        PredictAndDrawTrajectory(StartLocation, ProjectileInitialVelocity);
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
    Params.OverrideGravityZ = bHasGravity ? GetWorld()->GetGravityZ() : 0;
    Params.TraceChannel = ECC_Visibility;
    Params.ActorsToIgnore.Add(GetOwner());

    FPredictProjectilePathResult Result;
    UGameplayStatics::PredictProjectilePath(this, Params, Result);
    {
        DotInstances->ClearInstances();

        const APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
        const bool bHasCamera = IsValid(CameraManager);
        const FVector FallbackFacing = -GetOwner()->GetActorForwardVector();
        const float Scale = DotSize / 100.f;

        for (int32 i = 0; i < Result.PathData.Num(); ++i)
        {
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

            if (bDrawDebug && i > 0)
            {
                DrawDebugLine(
                    GetWorld(),
                    Result.PathData[i - 1].Location,
                    Result.PathData[i].Location,
                    DebugColor,
                    bDebugPersistentLines,
                    DebugLifeTime,
                    DebugDepthPriority,
                    DebugThickness
                );
            }
        }
    }
}
