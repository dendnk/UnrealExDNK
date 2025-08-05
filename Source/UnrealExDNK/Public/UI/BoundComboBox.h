// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "BoundWidgetInputBase.h"
#include "Components/ComboBoxString.h"
#include "BoundComboBox.generated.h"

UCLASS(Abstract, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALEXDNK_API UBoundComboBox
    : public UBoundWidgetInputBase
{
	GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UComboBoxString* ComboBox;

    virtual void NativeConstruct() override
    {
        Super::NativeConstruct();
        if (ComboBox)
        {
            ComboBox->OnSelectionChanged.AddDynamic(this, &ThisClass::HandleSelectionChanged);
        }
    }

    virtual void NativeDestruct() override
    {
        if (ComboBox)
        {
            ComboBox->OnSelectionChanged.RemoveAll(this);
        }
        Super::NativeDestruct();
    }

    UFUNCTION()
    void HandleSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
    {
        ReportErrorIfInvalid();
        if (WidgetInputReceiver)
        {
            WidgetInputReceiver->Execute_OnEnumSelectionChanged(WidgetInputReceiver.GetObject(), BoundPropertyName, SelectedItem, SeletionType);
        }
    }
};
