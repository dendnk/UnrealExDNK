// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "UIWidgetComponent.h"
#include "PropertyEditorComponent.generated.h"

class FProperty;

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
        UE_LOG(LogTemp, Log, TEXT("%s (%s): %s"), *Name, *Type, *Value);
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

#if WITH_EDITOR
public:
    UFUNCTION(BlueprintCallable)
    void GetEditableProperties(TArray<FPropertyInfo>& OutProperties);

    UFUNCTION(BlueprintCallable)
    bool ApplyPropertyValue(UObject* TargetObject, const FString& PropertyName, const FString& NewValueAsString);

    static bool IsPropertyEditable(FProperty* Property);
#endif
};
