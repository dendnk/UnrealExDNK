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

void UWeaponComponentBaseWidget::UpdateViewModelFromUI()
{
	if (IsValid(ViewModel) == false)
	{
		UE_DNK_LOG(LogTemp, Error, "ViewModel is invalid!");
		return;
	}

	const int64 DesiredFireTypeIndex = StaticEnum<EFireType>()->GetValueByNameString(FireTypeComboBox->GetSelectedOption());
	if (DesiredFireTypeIndex != INDEX_NONE)
	{
		ViewModel->SetFireType(static_cast<EFireType>(DesiredFireTypeIndex));
	}
	else
	{
		UE_DNK_LOG(LogTemp, Warning, "Check value in FireTypeComboBox! FireType will not be applied!");
	}

	const int64 DesiredFiringModeIndex = StaticEnum<EFiringMode>()->GetValueByNameString(FiringModeComboBox->GetSelectedOption());
	if (DesiredFiringModeIndex != INDEX_NONE)
	{
		ViewModel->SetFiringMode(static_cast<EFiringMode>(DesiredFiringModeIndex));
	}
	else
	{
		UE_DNK_LOG(LogTemp, Warning, "Check value in FiringModeComboBox! FiringMode will not be applied!");
	}

	ViewModel->SetMaxAmmo(FCString::Atof(*MaxAmmoTextBox->GetText().ToString()));
	ViewModel->SetAmmoPerShot(FCString::Atof(*AmmoPerShotTextBox->GetText().ToString()));
	ViewModel->SetCurrentAmmo(FCString::Atof(*CurrentAmmoTextBox->GetText().ToString()));
	ViewModel->SetIsInfiniteAmmo(InfiniteAmmoCheckBox->IsChecked());
	ViewModel->SetCooldownTime(FCString::Atof(*CooldownTimeTextBox->GetText().ToString()));
	ViewModel->SetIsNeedReload(NeedsReloadCheckBox->IsChecked());
	ViewModel->SetReloadTime(FCString::Atof(*ReloadTimeTextBox->GetText().ToString()));
	ViewModel->SetBaseDamage(FCString::Atof(*BaseDamageTextBox->GetText().ToString()));
	ViewModel->SetDamagePerTick(FCString::Atof(*DamagePerTickTextBox->GetText().ToString()));
	ViewModel->SetHitscanRange(FCString::Atof(*HitscanRangeTextBox->GetText().ToString()));
	ViewModel->SetHitscanSpread(FCString::Atof(*HitscanSpreadTextBox->GetText().ToString()));
	ViewModel->SetProjectileSpeed(FCString::Atof(*ProjectileSpeedTextBox->GetText().ToString()));
	ViewModel->SetBeamDuration(FCString::Atof(*BeamDurationTextBox->GetText().ToString()));
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
		{ TEXT("InfiniteAmmoCheckBox"), InfiniteAmmoCheckBox },
		{ TEXT("BurstCountTextBox"), BurstCountTextBox },
		{ TEXT("CooldownTimeTextBox"), CooldownTimeTextBox },
		{ TEXT("NeedsReloadCheckBox"), NeedsReloadCheckBox },
		{ TEXT("ReloadTimeTextBox"), ReloadTimeTextBox },
		{ TEXT("BaseDamageTextBox"), BaseDamageTextBox },
		{ TEXT("DamagePerTickTextBox"), DamagePerTickTextBox },
		{ TEXT("HitscanRangeTextBox"), HitscanRangeTextBox },
		{ TEXT("HitscanSpreadTextBox"), HitscanSpreadTextBox },
		{ TEXT("ProjectileSpeedTextBox"), ProjectileSpeedTextBox },
		{ TEXT("BeamDurationTextBox"), BeamDurationTextBox }
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
	BurstCountTextBox->SetText(FText::FromString(FString::SanitizeFloat(ViewModel->GetBurstCount())));
	CooldownTimeTextBox->SetText(FText::FromString(FString::SanitizeFloat(ViewModel->GetCooldownTime())));
	NeedsReloadCheckBox->SetIsChecked(ViewModel->IsNeedReload());
	ReloadTimeTextBox->SetText(FText::FromString(FString::SanitizeFloat(ViewModel->GetReloadTime())));
	BaseDamageTextBox->SetText(FText::FromString(FString::SanitizeFloat(ViewModel->GetBaseDamage())));
	DamagePerTickTextBox->SetText(FText::FromString(FString::SanitizeFloat(ViewModel->GetDamagePerTick())));
	HitscanRangeTextBox->SetText(FText::FromString(FString::SanitizeFloat(ViewModel->GetHitscanRange())));
	HitscanSpreadTextBox->SetText(FText::FromString(FString::SanitizeFloat(ViewModel->GetHitscanSpread())));
	ProjectileSpeedTextBox->SetText(FText::FromString(FString::SanitizeFloat(ViewModel->GetProjectileSpeed())));
	BeamDurationTextBox->SetText(FText::FromString(FString::SanitizeFloat(ViewModel->GetBeamDuration())));

	// Bind selection changed event
	FireTypeComboBox->OnSelectionChanged.AddDynamic(this, &ThisClass::OnFireTypeChanged);
	FiringModeComboBox->OnSelectionChanged.AddDynamic(this, &ThisClass::OnFiringModeChanged);
	MaxAmmoTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnMaxAmmoCommitted);
	AmmoPerShotTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnAmmoPerShotCommitted);
	CurrentAmmoTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnCurrentAmmoCommitted);
	InfiniteAmmoCheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::OnInfiniteAmmoChanged);
	BurstCountTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnBurstCountCommitted);
	CooldownTimeTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnCooldownTimeCommitted);
	NeedsReloadCheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::OnNeedsReloadChanged);
	ReloadTimeTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnReloadTimeCommitted);
	BaseDamageTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnBaseDamageCommitted);
	DamagePerTickTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnDamagePerTickCommitted);
	HitscanRangeTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnHitscanRangeCommitted);
	HitscanSpreadTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnHitscanSpreadCommitted);
	ProjectileSpeedTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnProjectileSpeedCommitted);
	BeamDurationTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnBeamDurationCommitted);
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

void UWeaponComponentBaseWidget::OnBurstCountCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ViewModel && Text.IsNumeric())
	{
		int32 Value = FCString::Atoi(*Text.ToString());
		ViewModel->SetBurstCount(Value);
	}
}

void UWeaponComponentBaseWidget::OnCooldownTimeCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ViewModel && Text.IsNumeric())
	{
		float Value = FCString::Atof(*Text.ToString());
		ViewModel->SetCooldownTime(Value);
	}
}

void UWeaponComponentBaseWidget::OnNeedsReloadChanged(bool bIsChecked)
{
	if (ViewModel)
	{
		ViewModel->SetIsNeedReload(bIsChecked);
	}
}

void UWeaponComponentBaseWidget::OnReloadTimeCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ViewModel && Text.IsNumeric())
	{
		float Value = FCString::Atof(*Text.ToString());
		ViewModel->SetReloadTime(Value);
	}
}

void UWeaponComponentBaseWidget::OnBaseDamageCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ViewModel && Text.IsNumeric())
	{
		float Value = FCString::Atof(*Text.ToString());
		ViewModel->SetBaseDamage(Value);
	}
}

void UWeaponComponentBaseWidget::OnDamagePerTickCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ViewModel && Text.IsNumeric())
	{
		float Value = FCString::Atof(*Text.ToString());
		ViewModel->SetDamagePerTick(Value);
	}
}

void UWeaponComponentBaseWidget::OnHitscanRangeCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ViewModel && Text.IsNumeric())
	{
		float Value = FCString::Atof(*Text.ToString());
		ViewModel->SetHitscanRange(Value);
	}
}

void UWeaponComponentBaseWidget::OnHitscanSpreadCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ViewModel && Text.IsNumeric())
	{
		float Value = FCString::Atof(*Text.ToString());
		ViewModel->SetHitscanSpread(Value);
	}
}

void UWeaponComponentBaseWidget::OnProjectileSpeedCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ViewModel && Text.IsNumeric())
	{
		float Value = FCString::Atof(*Text.ToString());
		ViewModel->SetProjectileSpeed(Value);
	}
}

void UWeaponComponentBaseWidget::OnBeamDurationCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (ViewModel && Text.IsNumeric())
	{
		float Value = FCString::Atof(*Text.ToString());
		ViewModel->SetBeamDuration(Value);
	}
}
