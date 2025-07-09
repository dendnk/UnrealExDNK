// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#include "LaserWeaponComponent.h"


ULaserWeaponComponent::ULaserWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void ULaserWeaponComponent::Fire()
{
    UE_LOG(LogTemp, Log, TEXT("Laser weapon fired!"));
    // Trace beam and apply damage over time
}
