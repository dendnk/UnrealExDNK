// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "EWeaponType.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Rocket		UMETA(DisplayName = "Rocket"),
	Rifle		UMETA(DisplayName = "Rifle"),
	Quadgun		UMETA(DisplayName = "Quadgun"),
	Shotgun		UMETA(DisplayName = "Shotgun"),
	Laser		UMETA(DisplayName = "Laser"),
	Custom		UMETA(DisplayName = "Custom")
};

