// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#include "Components/MachineGunComponent.h"


UMachineGunComponent::UMachineGunComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UMachineGunComponent::BeginPlay()
{
    Super::BeginPlay();

    if (bIsAutoFire)
    {
        StartFire();
    }
}
