// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "WeaponComponentBase.h"
#include "URocketLauncherComponent.generated.h"


/**
 * Rocket launcher component that fires projectile-based rockets.
 * Inherits from UWeaponComponentBase for modular weapon behavior.
 */
UCLASS(Blueprintable, ClassGroup = (Weapons), meta = (BlueprintSpawnableComponent))
class UNREALEXDNK_API URocketLauncherComponent : public UWeaponComponentBase
{
    GENERATED_BODY()

public:
    URocketLauncherComponent();

    virtual void Fire() override;

protected:
    virtual void BeginPlay() override;

    /** Class of the rocket projectile to spawn */
    UPROPERTY(EditAnywhere, Category = "Rocket")
    TSubclassOf<AActor> RocketClass;

    /** Optional socket name to spawn the rocket from */
    UPROPERTY(EditAnywhere, Category = "Rocket")
    FName MuzzleSocketName;

    /** Initial speed or impulse applied to the rocket */
    UPROPERTY(EditAnywhere, Category = "Rocket")
    float LaunchSpeed = 1500.0f;
};
