// Fill out your copyright notice in the Description page of Project Settings.


#include "IntersectionToolsStatics.h"
#include "Components/SpotLightComponent.h"

bool UIntersectionToolsStatics::AreSpotLightsOverlapping(const ASpotLight* SpotLightA, const ASpotLight* SpotLightB)
{
	return AreSpotLightComponentsOverlapping(SpotLightA->SpotLightComponent, SpotLightB->SpotLightComponent);
}

bool UIntersectionToolsStatics::AreSpotLightComponentsOverlapping(const USpotLightComponent* SpotLightComponentA, const USpotLightComponent* SpotLightComponentB)
{
	FBoxSphereBounds SpotLightBBounds = GetBoundingSphereForSpotLight(SpotLightComponentB);
	return IsSpotLightAffectBounds(SpotLightComponentA, SpotLightBBounds);
}

FSphere UIntersectionToolsStatics::GetBoundingSphereForSpotLight(const USpotLightComponent* SpotLightComponent)
{
	const float ClampedInnerConeAngle = FMath::Clamp(SpotLightComponent->InnerConeAngle, 0.0f, 89.0f) * (float)UE_PI / 180.0f;
	const float ClampedOuterConeAngle = FMath::Clamp(SpotLightComponent->OuterConeAngle * (float)UE_PI / 180.0f, ClampedInnerConeAngle + 0.001f, 89.0f * (float)UE_PI / 180.0f + 0.001f);

	FSphere::FReal ConeAngle = ClampedOuterConeAngle;
	FSphere::FReal CosConeAngle = FMath::Cos(ConeAngle);
	FSphere::FReal SinConeAngle = FMath::Sin(ConeAngle);
	return FMath::ComputeBoundingSphereForCone(
					SpotLightComponent->GetComponentTransform().GetLocation(),
					SpotLightComponent->GetDirection(),
					(FSphere::FReal)SpotLightComponent->AttenuationRadius, CosConeAngle, SinConeAngle);
}

bool UIntersectionToolsStatics::IsSpotLightAffectBounds(const USpotLightComponent* SpotLightComponent, const FBoxSphereBounds Bounds)
{
	if (SpotLightComponent->IsValidLowLevel())
	{
		return SpotLightComponent->AffectsBounds(Bounds);
	}

	return false;
}


