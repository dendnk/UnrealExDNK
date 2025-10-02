// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "TrajectoryPredictionComponent.generated.h"

class IWeaponUserInterface;
class UMaterialInterface;
class USplineMeshComponent;
class USplineComponent;
class UStaticMesh;
class UWeaponComponentBase;

UCLASS(Blueprintable, ClassGroup = (Weapons), meta = (BlueprintSpawnableComponent))
class MODULARWEAPONSYSTEM_API UTrajectoryPredictionComponent
    : public UActorComponent
{
    GENERATED_BODY()

    UTrajectoryPredictionComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void PredictAndDrawTrajectory(const FVector& StartLocation, const FVector& LaunchVelocity);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float MaxSimTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    int32 MaxSteps = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float TrajectoryScale = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float ProjectileRadius = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    bool bHasGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    TObjectPtr<UStaticMesh> ArcSegmentMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    TObjectPtr<UMaterialInterface> ArcSegmentMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory | Debug")
    bool bDrawDebug = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory | Debug")
    FColor DebugColor = FColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory | Debug")
    bool bDebugPersistentLines = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory | Debug")
    float DebugLifeTime = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory | Debug")
    uint8 DebugDepthPriority = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory | Debug")
    float DebugThickness = 2.0f;

private:
    void ClearArcMeshSegments();

    TObjectPtr<USplineComponent> TrajectorySpline;
    TArray<TObjectPtr<USplineMeshComponent>> ArcMeshSegments;
    TWeakObjectPtr<UWeaponComponentBase> Weapon;
    TWeakObjectPtr<USceneComponent> WeaponParentComponent;

    FName TrajectorySplineName = FName(TEXT("TrajectorySpline"));
};
