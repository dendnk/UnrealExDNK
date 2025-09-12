// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "UObject/Interface.h"
#include "IDamageableInterface.generated.h"

UINTERFACE(Blueprintable)
class MODULARWEAPONSYSTEM_API UDamageableInterface : public UInterface
{
    GENERATED_BODY()
};

class MODULARWEAPONSYSTEM_API IDamageableInterface
{
    GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	void HandleDamage(float DamageAmount, AActor* DamageCauser);
};
