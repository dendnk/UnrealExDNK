// Copyright Zapilili Games. All Rights Reserved.

#pragma once

#include "UIWidgetComponent.h"
#include "UnrealExDNKUtils.h"
#include "UObject/UnrealType.h"
#include "PropertyEditorComponent.generated.h"


USTRUCT(BlueprintType)
struct FPropertyInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    FString Type;

    UPROPERTY(BlueprintReadWrite)
    FString Value;

    void Log() const
    {
        UE_DNK_LOG(LogTemp, Log, TEXT("%s (%s): %s"), *Name, *Type, *Value);
    }
};

/**
 *	Reflects and shows current property values of an Actor and its Components
 *  Allows users (via UI) to edit values at runtime
 *  Applies changes back to the actor/components
 */
UCLASS()
class UNREALEXDNK_API UPropertyEditorComponent : public UUIWidgetComponent
{
	GENERATED_BODY()
	
public:
    // Get properties of this actor (and optionally its components)
    UFUNCTION(BlueprintCallable)
    void GetEditableProperties(TArray<FPropertyInfo>& OutProperties);

    // Apply new value to a given property
    UFUNCTION(BlueprintCallable)
    bool ApplyPropertyValue(UObject* TargetObject, FProperty* Property, const FString& NewValueAsString);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    static bool IsPropertyEditable(FProperty* Property);
};
