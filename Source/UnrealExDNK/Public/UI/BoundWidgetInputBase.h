// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "IWidgetInputReceiver.h"
#include "BoundWidgetInputBase.generated.h"

UCLASS(Abstract, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALEXDNK_API UBoundWidgetInputBase
    : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY()
    FName BoundPropertyName;

    UPROPERTY()
    TScriptInterface<IWidgetInputReceiver> WidgetInputReceiver;

protected:
    virtual void NativeDestruct() override
    {
        WidgetInputReceiver = nullptr;
        Super::NativeDestruct();
    }

    void ReportErrorIfInvalid() const
    {
        if (!WidgetInputReceiver)
        {
            UE_LOG(LogTemp, Error, TEXT("[%s] WidgetInputReceiver is INVALID!"), *GetName());
        }
    }
};
