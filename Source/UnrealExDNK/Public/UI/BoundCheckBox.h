// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "BoundWidgetInputBase.h"
#include "Components/CheckBox.h"
#include "BoundCheckBox.generated.h"

UCLASS(Abstract, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALEXDNK_API UBoundCheckBox
    : public UBoundWidgetInputBase
{
	GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UCheckBox* CheckBox;

    virtual void NativeConstruct() override
    {
        Super::NativeConstruct();
        if (CheckBox)
        {
            CheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::HandleCheckStateChanged);
        }
    }

    virtual void NativeDestruct() override
    {
        if (CheckBox)
        {
            CheckBox->OnCheckStateChanged.RemoveAll(this);
        }
        Super::NativeDestruct();
    }

    UFUNCTION()
    void HandleCheckStateChanged(bool bNewState)
    {
        ReportErrorIfInvalid();
        if (WidgetInputReceiver)
        {
            WidgetInputReceiver->Execute_OnCheckStateChanged(WidgetInputReceiver.GetObject(), BoundPropertyName, bNewState);
        }
    }
};
