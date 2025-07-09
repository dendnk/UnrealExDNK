// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#include "QuadGunComponent.h"


UQuadGunComponent::UQuadGunComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UQuadGunComponent::Fire()
{
    UE_LOG(LogTemp, Log, TEXT("Quad gun fired 4 bullets!"));
    // Fire 4 bullets either in burst or from 4 separate muzzles
}
