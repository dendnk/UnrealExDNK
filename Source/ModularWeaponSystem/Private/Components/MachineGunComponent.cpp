// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#include "Components/MachineGunComponent.h"


UMachineGunComponent::UMachineGunComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UMachineGunComponent::Fire()
{
    UE_LOG(LogTemp, Log, TEXT("Machine gun fired!"));
    // Add bullet spawning and spread logic here
}

void UMachineGunComponent::StartFiring()
{
}

void UMachineGunComponent::StopFiring()
{
}
