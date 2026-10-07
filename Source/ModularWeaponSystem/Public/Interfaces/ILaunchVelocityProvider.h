// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "UObject/Interface.h"
#include "ILaunchVelocityProvider.generated.h"

UINTERFACE(NotBlueprintable, MinimalAPI)
class ULaunchVelocityProvider : public UInterface
{
    GENERATED_BODY()
};

/**
 * Opt-in hook for weapons whose real projectile launch velocity differs from
 * ProjectileSpeed * ShotDirection (e.g. the player rocket launcher adds the helicopter's
 * velocity). UTrajectoryPredictionComponent uses it so the aim dots match the real flight;
 * weapons that don't implement it keep the default preview logic unchanged.
 */
class MODULARWEAPONSYSTEM_API ILaunchVelocityProvider
{
    GENERATED_BODY()

public:
    /** Velocity a projectile fired along ShotDirection (normalized, already lane-flattened) would launch with. */
    virtual FVector GetPredictedLaunchVelocity(const FVector& ShotDirection) const = 0;
};
