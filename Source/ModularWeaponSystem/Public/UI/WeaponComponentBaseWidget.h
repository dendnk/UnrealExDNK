// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "WeaponComponentBaseWidget.generated.h"

class UComboBoxString;
class UWeaponViewModel;

/**
 * Widget for showing weapon component parameters
 */
UCLASS()
class MODULARWEAPONSYSTEM_API UWeaponComponentBaseWidget
	: public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void SetViewModel(UWeaponViewModel* InViewModel);


protected:
	UFUNCTION()
	void UpdateUIFromViewModel();

	// Callback for user changing the selected item
	UFUNCTION()
	void OnFiringModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	// Helper to populate options
	void PopulateFiringModeOptions();

	// ComboBox for selecting FiringMode
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* FiringModeComboBox;

	UPROPERTY(Transient, meta = (BindWidgetViewModel))
	TObjectPtr<UWeaponViewModel> ViewModel;
};
