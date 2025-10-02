// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "Components/TrajectoryPredictionComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/WeaponComponentBase.h"
#include "Interfaces/IWeaponUserInterface.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealExDNKUtils.h"


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
        UE_LOG(LogTemp, Error, TEXT("GetOwner == false || GetWorld == false!"));
        return;
    }

    TArray<UWeaponComponentBase*> WeaponWithProjectiles =
        ActorComponents::GetComponentsByCondition<UWeaponComponentBase>(Owner,
            [](const UWeaponComponentBase* Weapon)
            {
                if (IsValid(Weapon))
                {
                    return Weapon->GetWeaponDataRuntime()->FireType == EFireType::Projectile;
                }

                return false;
            });

    Weapon = WeaponWithProjectiles.Num() > 0 ? WeaponWithProjectiles[0] : nullptr;
    if (Weapon.IsValid() == false)
    {
        UE_LOG(LogTemp, Error, TEXT("No Weapon found on %s"), *GetOwner()->GetName());
        SetComponentTickEnabled(false);
        return;
    }

    WeaponParentComponent = IWeaponUserInterface::Execute_GetParentAttachment(Weapon->GetOwner());

    TrajectorySpline = NewObject<USplineComponent>(Owner, USplineComponent::StaticClass(), TrajectorySplineName);
    if (TrajectorySpline)
    {
        TrajectorySpline->RegisterComponent();
        TrajectorySpline->AttachToComponent(WeaponParentComponent.Get(), FAttachmentTransformRules::KeepRelativeTransform);
    }

    Super::BeginPlay();
}

void UTrajectoryPredictionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    if (Weapon.IsValid())
    {
        const FVector StartLocation = Weapon->GetMuzzleTransform().GetLocation();
        const FVector ProjectileForwardDirection = Weapon->GetMuzzleTransform().GetRotation().GetForwardVector();
        const FVector InheritedVelocity = -GetOwner()->GetVelocity();
        const FVector ProjectileInitialVelocity = Weapon->GetWeaponDataAsset()->ProjectileSpeed * ProjectileForwardDirection;

        //FPredictProjectilePathParams Params = Config.RocketAimTrajectorySetup.Params;
        //Params.StartLocation = StartLocation;
        //Params.LaunchVelocity = InheritedVelocity + ProjectileInitialVelocity;
        //Params.OverrideGravityZ = GetWorld()->GetGravityZ();
        //Params.ActorsToIgnore.Add(GetOwner());

        PredictAndDrawTrajectory(StartLocation, ProjectileInitialVelocity);
    }

    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTrajectoryPredictionComponent::PredictAndDrawTrajectory(const FVector& StartLocation, const FVector& LaunchVelocity)
{
    if (IsValid(GetOwner()) == false ||
        IsValid(GetWorld()) == false ||
        Weapon.IsValid() == false ||
        WeaponParentComponent.IsValid() == false)
    {
        UE_LOG(LogTemp, Error, TEXT(
            "GetOwner == false || GetWorld == false! || Weapon.IsValid() == false || WeaponParentComponent.IsValid() == false"));
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
        ClearArcMeshSegments();
        const bool bShouldUpdateSpline = false;
        TrajectorySpline->ClearSplinePoints(bShouldUpdateSpline);

        for (int32 i = 0; i < Result.PathData.Num(); ++i)
        {
            TrajectorySpline->AddSplinePoint(Result.PathData[i].Location, ESplineCoordinateSpace::World, false);
        }

        TrajectorySpline->UpdateSpline();

        const int32 NumSegments = Result.PathData.Num() - 1;
        for (int32 i = 0; i < NumSegments; ++i)
        {
            FVector StartPos, StartTangent, EndPos, EndTangent;
            FVector2D StartScale, EndScale;

            StartPos = TrajectorySpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
            StartTangent = TrajectorySpline->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::World);
            StartScale = FVector2D(TrajectoryScale);

            EndPos = TrajectorySpline->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World);
            EndTangent = TrajectorySpline->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::World);
            EndScale = StartScale;

            USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
            SplineMesh->SetStaticMesh(ArcSegmentMesh);
            SplineMesh->SetMaterial(0, ArcSegmentMaterial);
            SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
            SplineMesh->SetStartScale(StartScale);
            SplineMesh->SetEndScale(EndScale);
            SplineMesh->SetMobility(EComponentMobility::Movable);
            SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            SplineMesh->RegisterComponentWithWorld(GetWorld());
            SplineMesh->AttachToComponent(WeaponParentComponent.Get(), FAttachmentTransformRules::KeepWorldTransform);

            ArcMeshSegments.Add(SplineMesh);

            if (bDrawDebug)
            {
                DrawDebugLine(
                    GetWorld(),
                    Result.PathData[i].Location,
                    Result.PathData[i + 1].Location,
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

void UTrajectoryPredictionComponent::ClearArcMeshSegments()
{
    for (USplineMeshComponent* Mesh : ArcMeshSegments)
    {
        if (Mesh)
        {
            Mesh->DestroyComponent();
        }
    }
    ArcMeshSegments.Empty();
}
