// Copyright 2025 [UnrealExDNK | Modular Weapon System : Denis Kruchok]. All rights reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "UI/IWidgetInputReceiver.h"
#include "WeaponComponentBaseWidget.generated.h"

class UBoundWidgetInputBase;
class UVerticalBox;
class UWeaponViewModel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSetViewModelDelegate, TWeakObjectPtr<UWeaponViewModel>, WeaponViewModel);

/**
 * Widget for showing weapon component parameters
 */
UCLASS()
class MODULARWEAPONSYSTEM_API UWeaponComponentBaseWidget
	: public UUserWidget
	, public IWidgetInputReceiver
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable)
	void SetViewModel(UWeaponViewModel* InViewModel);

	UFUNCTION(BlueprintCallable)
	void UpdateViewModelFromUI();

	UPROPERTY(BlueprintAssignable)
	FOnSetViewModelDelegate OnSetViewModel;

	UPROPERTY(BlueprintAssignable)
	FOnSetViewModelDelegate OnUpdateViewModelFromUI;


protected:
	void OnEnumSelectionChanged(FName PropertyName, const FString& SelectedItem);
	void OnCheckStateChanged(FName PropertyName, bool bIsChecked);
	void OnTextCommitted(FName PropertyName, const FText& NewText, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void UpdateUIFromViewModel();


	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PropertiesVerticalBox;

	UPROPERTY(Transient, meta = (BindWidgetViewModel))
	TWeakObjectPtr<UWeaponViewModel> ViewModel;

private:
	TMap<FName, TObjectPtr<UBoundWidgetInputBase>> PropertyWidgets;
};
