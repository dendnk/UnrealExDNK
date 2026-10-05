#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FTerrainGenerator : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
