// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "WeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	RocketLauncher		UMETA(DisplayName = "RocketLauncher"),
	MachineGun			UMETA(DisplayName = "MachineGun"),
	Quadgun				UMETA(DisplayName = "Quadgun"),
	Shotgun				UMETA(DisplayName = "Shotgun"),
	Laser				UMETA(DisplayName = "Laser"),
	Custom				UMETA(DisplayName = "Custom")
};
