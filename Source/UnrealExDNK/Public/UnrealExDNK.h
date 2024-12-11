// Copyright 2024 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FUnrealExDNKModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
