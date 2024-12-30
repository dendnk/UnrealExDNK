// Copyright 2024 [HeliAce: Denis Kruchok, Denis Romanovsky]. All rights reserved.

#pragma once

#include "UObject/Interface.h"
#include "OwningActorInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UOwningActorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
* Interface for retrieving the actor that owns or is associated with the implementing object.
*/
class UNREALEXDNK_API IOwningActorInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = OwningActor)
	AActor* GetOwningActor() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = OwningActor)
	void SetOwningActor(AActor* InOwningActor);
};
