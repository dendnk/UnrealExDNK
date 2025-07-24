// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.


#include "UI/WeaponComponentBaseWidget.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
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

	OnSetViewModel.Broadcast(ViewModel);
}

void UWeaponComponentBaseWidget::UpdateUIFromViewModel()
{
	if (IsValid(ViewModel) == false)
	{
		UE_DNK_LOG(LogTemp, Error, "ViewModel is invalid!");
		return;
	}

	bool bAllValid = true;

	const TArray<TPair<FString, UObject*>> WidgetChecks = {
		{ TEXT("FireTypeComboBox"), FireTypeComboBox },
		{ TEXT("FiringModeComboBox"), FiringModeComboBox },
		{ TEXT("MaxAmmoTextBox"), MaxAmmoTextBox },
		{ TEXT("AmmoPerShotTextBox"), AmmoPerShotTextBox },
		{ TEXT("CurrentAmmoTextBox"), CurrentAmmoTextBox },
		{ TEXT("InfiniteAmmoCheckBox"), InfiniteAmmoCheckBox }
	};

	for (const auto& [Name, Widget] : WidgetChecks)
	{
		if (IsValid(Widget) == false)
		{
			bAllValid = false;
			UE_DNK_LOG(LogTemp, Error, "%s is Invalid!", *Name);
		}
	}

	if (!bAllValid)
	{
		UE_DNK_LOG(LogTemp, Error, "Some of the widgets were not found. Initialization aborted.");
		return;
	}

	PopulateEnumOptions<EFireType>(FireTypeComboBox);
	PopulateEnumOptions<EFiringMode>(FiringModeComboBox);

	FireTypeComboBox->SetSelectedOption(ViewModel->GetFireTypeAsString());
	FiringModeComboBox->SetSelectedOption(ViewModel->GetFiringModeAsString());
	MaxAmmoTextBox->SetText(FText::FromString(FString::FromInt(ViewModel->GetMaxAmmo())));
	AmmoPerShotTextBox->SetText(FText::FromString(FString::FromInt(ViewModel->GetAmmoPerShot())));
	CurrentAmmoTextBox->SetText(FText::FromString(FString::FromInt(ViewModel->GetCurrentAmmo())));
	InfiniteAmmoCheckBox->SetIsChecked(ViewModel->IsInfiniteAmmo());

	// Bind selection changed event
	FireTypeComboBox->OnSelectionChanged.AddDynamic(this, &ThisClass::OnFireTypeChanged);
	FiringModeComboBox->OnSelectionChanged.AddDynamic(this, &ThisClass::OnFiringModeChanged);
	MaxAmmoTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnMaxAmmoCommitted);
	AmmoPerShotTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnAmmoPerShotCommitted);
	CurrentAmmoTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnCurrentAmmoCommitted);
	InfiniteAmmoCheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::OnInfiniteAmmoChanged);
}

void UWeaponComponentBaseWidget::OnFireTypeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	SetEnumFieldSafe<EFireType, UWeaponViewModel>(ViewModel, SelectedItem,
		[](UWeaponViewModel* WeaponViewModel, EFireType FireType)
		{
			WeaponViewModel->SetFireType(FireType);
		});
}

void UWeaponComponentBaseWidget::OnFiringModeChanged(FString SelectedItem, ESelectInfo::Type)
{
	SetEnumFieldSafe<EFiringMode, UWeaponViewModel>(ViewModel, SelectedItem,
		[](UWeaponViewModel* WeaponViewModel, EFiringMode FiringMode)
		{
			WeaponViewModel->SetFiringMode(FiringMode);
		});
}

void UWeaponComponentBaseWidget::OnMaxAmmoCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ViewModel && Text.IsNumeric())
	{
		int32 Value = FCString::Atoi(*Text.ToString());
		ViewModel->SetMaxAmmo(Value);
	}
}

void UWeaponComponentBaseWidget::OnAmmoPerShotCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ViewModel && Text.IsNumeric())
	{
		int32 Value = FCString::Atoi(*Text.ToString());
		ViewModel->SetAmmoPerShot(Value);
	}
}

void UWeaponComponentBaseWidget::OnCurrentAmmoCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ViewModel && Text.IsNumeric())
	{
		int32 Value = FCString::Atoi(*Text.ToString());
		ViewModel->SetCurrentAmmo(Value);
	}
}

void UWeaponComponentBaseWidget::OnInfiniteAmmoChanged(bool bIsChecked)
{
	if (ViewModel)
	{
		ViewModel->SetIsInfiniteAmmo(bIsChecked);
	}
}
