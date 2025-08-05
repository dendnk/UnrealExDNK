// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "BoundWidgetInputBase.h"
#include "Components/EditableTextBox.h"
#include "BoundEditableTextBox.generated.h"

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALEXDNK_API UBoundEditableTextBox
    : public UBoundWidgetInputBase
{
	GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* TextBox;

    virtual void NativeConstruct() override
    {
        Super::NativeConstruct();
        if (TextBox)
        {
            TextBox->OnTextCommitted.AddDynamic(this, &ThisClass::HandleTextCommitted);
        }
    }

    virtual void NativeDestruct() override
    {
        if (TextBox)
        {
            TextBox->OnTextCommitted.RemoveAll(this);
        }
        Super::NativeDestruct();
    }

    UFUNCTION()
    void HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
    {
        ReportErrorIfInvalid();
        if (WidgetInputReceiver)
        {
            WidgetInputReceiver->Execute_OnTextCommitted(WidgetInputReceiver.GetObject(), BoundPropertyName, Text, CommitMethod);
        }
    }
};
