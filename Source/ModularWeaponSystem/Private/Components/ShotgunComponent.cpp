// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#include "Components/ShotgunComponent.h"


UShotgunComponent::UShotgunComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UShotgunComponent::Fire()
{
    // Spawn multiple projectiles with spread
}
