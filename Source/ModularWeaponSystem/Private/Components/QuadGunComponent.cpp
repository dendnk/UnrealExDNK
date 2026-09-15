// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#include "Components/QuadGunComponent.h"


UQuadGunComponent::UQuadGunComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UQuadGunComponent::Fire()
{
    // Fire 4 bullets either in burst or from 4 separate muzzles
}
