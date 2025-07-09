// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#include "ModularWeaponSystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogModularWeaponSystem, Log, All);

#define LOCTEXT_NAMESPACE "FModularWeaponSystemModule"

void FModularWeaponSystemModule::StartupModule()
{
	UE_LOG(LogModularWeaponSystem, Log, TEXT("ModularWeaponSystem module has started."));
}

void FModularWeaponSystemModule::ShutdownModule()
{
	UE_LOG(LogModularWeaponSystem, Log, TEXT("ModularWeaponSystem module has shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModularWeaponSystemModule, ModularWeaponSystem)
