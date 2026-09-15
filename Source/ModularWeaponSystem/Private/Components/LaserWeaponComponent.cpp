// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#include "Components/LaserWeaponComponent.h"


ULaserWeaponComponent::ULaserWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void ULaserWeaponComponent::Fire()
{
    // Trace beam and apply damage over time
}
