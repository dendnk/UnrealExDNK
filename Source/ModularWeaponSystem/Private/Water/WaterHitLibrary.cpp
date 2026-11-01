// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#include "Water/WaterHitLibrary.h"

#include "Components/PrimitiveComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

const FName UWaterHitLibrary::WaterTag(TEXT("Water"));

bool UWaterHitLibrary::IsWaterComponent(const UPrimitiveComponent* Component)
{
	return IsValid(Component) && Component->ComponentHasTag(WaterTag);
}

bool UWaterHitLibrary::IsWaterHit(const FHitResult& Hit)
{
	return IsWaterComponent(Hit.GetComponent());
}

FVector UWaterHitLibrary::ProjectOntoWaterSurface(const UPrimitiveComponent* WaterComponent, const FVector& Location)
{
	if (!IsValid(WaterComponent))
	{
		return Location;
	}

	const FBoxSphereBounds& Bounds = WaterComponent->Bounds;
	return FVector(Location.X, Location.Y, Bounds.Origin.Z + Bounds.BoxExtent.Z);
}

UNiagaraComponent* UWaterHitLibrary::SpawnWaterEffect(const UObject* WorldContextObject, UNiagaraSystem* System,
                                                      const FVector& Location)
{
	if (System == nullptr || WorldContextObject == nullptr)
	{
		return nullptr;
	}

	return UNiagaraFunctionLibrary::SpawnSystemAtLocation(WorldContextObject, System, Location, FRotator::ZeroRotator,
		FVector(1.0f), /*bAutoDestroy=*/true, /*bAutoActivate=*/true, ENCPoolMethod::AutoRelease);
}
