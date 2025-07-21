// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.


#include "UI/WeaponComponentBaseWidget.h"
#include "Components/ComboBoxString.h"
#include "UI/WeaponViewModel.h"
#include "UnrealExDNKUtils.h"
#include "UObject/Class.h"
#include "View/MVVMView.h"


void UWeaponComponentBaseWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UWeaponComponentBaseWidget::SetViewModel(UWeaponViewModel* InViewModel)
{
	if (IsValid(InViewModel) == false)
	{
		UE_DNK_LOG(LogTemp, Error, "InViewModel is invalid!");
		return;
	}

	ViewModel = InViewModel;
	UpdateUIFromViewModel();
}

void UWeaponComponentBaseWidget::UpdateUIFromViewModel()
{
	if (IsValid(FiringModeComboBox))
	{
		PopulateFiringModeOptions();

		// Bind selection changed event
		FiringModeComboBox->OnSelectionChanged.AddDynamic(this, &ThisClass::OnFiringModeChanged);

		// Optional: set initial selection
		if (IsValid(ViewModel))
		{
			FString Current = ViewModel->GetFiringModeAsString();
			FiringModeComboBox->SetSelectedOption(Current);
		}
	}
}

void UWeaponComponentBaseWidget::PopulateFiringModeOptions()
{
	if (!FiringModeComboBox) return;

	FiringModeComboBox->ClearOptions();
	UEnum* Enum = StaticEnum<EFiringMode>();
	if (!Enum) return;

	for (int32 i = 0; i < Enum->NumEnums() - 1; ++i)
	{
		FString Option = Enum->GetNameStringByIndex(i);
		FiringModeComboBox->AddOption(Option);
	}
}

void UWeaponComponentBaseWidget::OnFiringModeChanged(FString SelectedItem, ESelectInfo::Type)
{
	if (!ViewModel) return;

	UEnum* Enum = StaticEnum<EFiringMode>();
	if (!Enum) return;

	int64 Value = Enum->GetValueByNameString(SelectedItem);
	if (Enum->IsValidEnumValue(Value))
	{
		ViewModel->SetFiringMode(static_cast<EFiringMode>(Value));
	}
}
