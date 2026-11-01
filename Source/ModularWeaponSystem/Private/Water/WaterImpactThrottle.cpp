// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#include "Water/WaterImpactThrottle.h"

#include "Engine/World.h"

bool UWaterImpactThrottle::TryConsume(const UObject* Key, float MinInterval)
{
	const UWorld* World = GetWorld();
	if (Key == nullptr || World == nullptr)
	{
		return true;
	}

	const double Now = World->GetTimeSeconds();
	double& LastSpawnTime = LastSpawnTimeByKey.FindOrAdd(FObjectKey(Key), -1.0e9);
	if (Now - LastSpawnTime < MinInterval)
	{
		return false;
	}

	LastSpawnTime = Now;
	return true;
}
