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

    /**
     * Two-phase flight of the real projectile: no gravity for OutStraightTime seconds, then gravity scaled by
     * OutFallGravityScale until OutLifeSpan. false = plain one-gravity flight (the preview keeps its MaxSimTime path).
     */
    virtual bool GetPredictedFlightProfile(float& OutStraightTime, float& OutFallGravityScale, float& OutLifeSpan) const
    {
        return false;
    }
};
