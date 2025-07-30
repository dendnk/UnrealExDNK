// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "WeaponComponentBaseWidget.generated.h"

class UCheckBox;
class UEditableTextBox;
class UWeaponViewModel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSetViewModelDelegate, UWeaponViewModel*, WeaponViewModel);

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

	UFUNCTION(BlueprintCallable)
	void UpdateViewModelFromUI();

	UPROPERTY(BlueprintAssignable)
	FOnSetViewModelDelegate OnSetViewModel;

	UPROPERTY(BlueprintAssignable)
	FOnSetViewModelDelegate OnUpdateViewModelFromUI;


protected:
	UFUNCTION()
	void UpdateUIFromViewModel();

	UFUNCTION()
	void OnFireTypeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnFiringModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnMaxAmmoCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnAmmoPerShotCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnCurrentAmmoCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnInfiniteAmmoChanged(bool bIsChecked);

	UFUNCTION()
	void OnBurstCountCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnCooldownTimeCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnNeedsReloadChanged(bool bIsChecked);

	UFUNCTION()
	void OnReloadTimeCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnBaseDamageCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnDamagePerTickCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnHitscanRangeCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnHitscanSpreadCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnProjectileSpeedCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnBeamDurationCommitted(const FText& Text, ETextCommit::Type CommitMethod);


	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> FireTypeComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> FiringModeComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> MaxAmmoTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> AmmoPerShotTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> CurrentAmmoTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> InfiniteAmmoCheckBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> BurstCountTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> CooldownTimeTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> NeedsReloadCheckBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> ReloadTimeTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> BaseDamageTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> DamagePerTickTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> HitscanRangeTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> HitscanSpreadTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> ProjectileSpeedTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> BeamDurationTextBox;


	UPROPERTY(Transient, meta = (BindWidgetViewModel))
	TObjectPtr<UWeaponViewModel> ViewModel;
};

template<typename TEnum, typename TViewModel>
void SetEnumFieldSafe(TObjectPtr<TViewModel> ViewModel, const FString& SelectedItem, TFunction<void(TViewModel*, TEnum)> Setter)
{
	static_assert(TIsDerivedFrom<TViewModel, UObject>::Value, "TViewModel must be a UObject");
	static_assert(TIsEnum<TEnum>::Value, "TEnum must be an enumeration type.");

	if (IsValid(ViewModel) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("ViewModel is invalid!"));
		return;
	}

	static_assert(TIsEnum<TEnum>::Value, "TEnum must be an enumeration type.");

	UEnum* Enum = StaticEnum<TEnum>();
	if (!Enum)
	{
		UE_LOG(LogTemp, Error, TEXT("Enum reflection missing for %s"), *FString(typeid(TEnum).name()));
		return;
	}

	int64 Value = Enum->GetValueByNameString(SelectedItem);
	if (Enum->IsValidEnumValue(Value))
	{
		Setter(ViewModel, static_cast<TEnum>(Value));
	}
}

template<typename TEnum>
void PopulateEnumOptions(UComboBoxString* ComboBox)
{
	if (IsValid(ComboBox) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("ComboBox is invalid!"));
		return;
	}

	ComboBox->ClearOptions();

	if (UEnum* Enum = StaticEnum<TEnum>())
	{
		for (int32 i = 0; i < Enum->NumEnums(); ++i)
		{
			if (!Enum->HasMetaData(TEXT("Hidden"), i))
			{
				FString Name = Enum->GetNameStringByIndex(i);
				ComboBox->AddOption(Name);
			}
		}
	}
}
