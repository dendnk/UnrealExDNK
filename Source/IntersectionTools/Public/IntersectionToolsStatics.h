// Copyright 2024 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Engine/SpotLight.h"

#include "IntersectionToolsStatics.generated.h"

UCLASS()
class INTERSECTIONTOOLS_API UIntersectionToolsStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = "IntersectionTools | Spot Lights", BlueprintPure)
	static bool AreSpotLightsOverlapping(const ASpotLight* SpotLightA, const ASpotLight* SpotLightB);

	UFUNCTION(Category = "IntersectionTools | Spot Lights", BlueprintPure)
	static bool AreSpotLightComponentsOverlapping(const USpotLightComponent* SpotLightComponentA, const USpotLightComponent* SpotLightComponentB);

	UFUNCTION(Category = "IntersectionTools | Spot Lights", BlueprintPure)
	static FSphere GetBoundingSphereForSpotLight(const USpotLightComponent* SpotLightComponent);

	UFUNCTION(Category = "IntersectionTools | Spot Lights", BlueprintPure)
	static bool IsSpotLightAffectBounds(const USpotLightComponent* SpotLightComponent, const FBoxSphereBounds Bounds);
};
