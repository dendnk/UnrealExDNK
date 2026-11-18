// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#include "Impact/WeaponSystemHitSettings.h"

#include "Engine/HitResult.h"
#include "GameFramework/Pawn.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

bool UWeaponSystemHitSettings::TrySpawnPlayerHit(const UObject* WorldContext, const FHitResult& Hit)
{
	const APawn* Pawn = Cast<APawn>(Hit.GetActor());
	if (WorldContext == nullptr || Pawn == nullptr || !Pawn->IsPlayerControlled())
	{
		return false;
	}

	const UWeaponSystemHitSettings* Settings = Get();
	UNiagaraSystem* Effect = Settings->PlayerHitEffect.LoadSynchronous();
	if (Effect == nullptr)
	{
		return false;
	}

	const FVector Point = Hit.ImpactPoint.IsNearlyZero() ? FVector(Hit.Location) : FVector(Hit.ImpactPoint);
	const FVector Normal = Hit.ImpactNormal.IsNearlyZero() ? FVector::UpVector : FVector(Hit.ImpactNormal);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(WorldContext, Effect, Point, Normal.Rotation(),
		FVector(Settings->PlayerHitEffectScale));
	return true;
}
