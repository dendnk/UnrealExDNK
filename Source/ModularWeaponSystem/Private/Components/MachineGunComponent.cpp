// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#include "MachineGunComponent.h"


UMachineGunComponent::UMachineGunComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UMachineGunComponent::Fire()
{
    UE_LOG(LogTemp, Log, TEXT("Machine gun fired!"));
    // Add bullet spawning and spread logic here
}
