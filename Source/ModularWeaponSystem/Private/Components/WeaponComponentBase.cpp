// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#include "Components/WeaponComponentBase.h"


void UWeaponComponentBase::BP_Fire_Implementation()
{
	Fire();
}

void UWeaponComponentBase::Fire()
{
}

bool UWeaponComponentBase::CanFire() const
{
	return false;
}
