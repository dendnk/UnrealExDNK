// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#include "ShotgunComponent.h"


UShotgunComponent::UShotgunComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UShotgunComponent::Fire()
{
    UE_LOG(LogTemp, Log, TEXT("Shotgun fired with multiple pellets!"));
    // Spawn multiple projectiles with spread
}
