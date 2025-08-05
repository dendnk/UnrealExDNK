// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "UObject/Interface.h"
#include "IWidgetInputReceiver.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UWidgetInputReceiver : public UInterface
{
    GENERATED_BODY()
};

class IWidgetInputReceiver
{
    GENERATED_BODY()

public:
    // Called when an enum ComboBox value changes
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void OnEnumSelectionChanged(FName PropertyName, const FString& SelectedItem, ESelectInfo::Type SelectionType);

    // Called when a CheckBox state changes
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void OnCheckStateChanged(FName PropertyName, bool bIsChecked);

    // Called when text changes in a UEditableTextBox (on commit or text change, your choice)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void OnTextCommitted(FName PropertyName, const FText& NewText, ETextCommit::Type CommitMethod);
};
