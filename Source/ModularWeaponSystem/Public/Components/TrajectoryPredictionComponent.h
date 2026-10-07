// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "TrajectoryPredictionComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTrajectoryPrediction, Log, All);

class IWeaponUserInterface;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
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

    // Finds the sibling projectile weapon and sets up DotInstances. Deferred to the first tick
    // (rather than run from BeginPlay) because sibling UActorComponent::BeginPlay() order on the
    // same actor is unspecified - WeaponComponentBase's WeaponDataRuntime is only populated in its
    // own BeginPlay, so looking it up from here in BeginPlay can race. By the first tick every
    // component's BeginPlay has already run. Runs at most once - see bInitializationAttempted.
    void EnsureInitialized();
    void PredictAndDrawTrajectory(const FVector& StartLocation, const FVector& LaunchVelocity);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float MaxSimTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    int32 MaxSteps = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float DotSize = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float ProjectileRadius = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    bool bHasGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    TObjectPtr<UStaticMesh> DotMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    TObjectPtr<UMaterialInterface> DotMaterial;

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
    TObjectPtr<UInstancedStaticMeshComponent> DotInstances;
    TWeakObjectPtr<UWeaponComponentBase> Weapon;
    TWeakObjectPtr<USceneComponent> WeaponParentComponent;
    bool bInitializationAttempted = false;

    FName DotInstancesName = FName(TEXT("TrajectoryDotInstances"));
};
