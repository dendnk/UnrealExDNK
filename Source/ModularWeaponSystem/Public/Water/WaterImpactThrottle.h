// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "UObject/ObjectKey.h"
#include "WaterImpactThrottle.generated.h"

// Per-weapon rate limit for bullet water puffs (HeliAce specs/implementing-water-surface-and-splash REQ-10), so a
// machine-gun burst into the sea doesn't spawn a puff per bullet.
UCLASS()
class MODULARWEAPONSYSTEM_API UWaterImpactThrottle : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// True, and records the spawn, if Key's last puff is at least MinInterval seconds ago.
	bool TryConsume(const UObject* Key, float MinInterval);

private:
	TMap<FObjectKey, double> LastSpawnTimeByKey;
};
