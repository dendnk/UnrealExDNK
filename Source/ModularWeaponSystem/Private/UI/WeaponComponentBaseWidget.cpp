// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.


#include "UI/WeaponComponentBaseWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/ScaleBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Projectiles/ProjectileBase.h"
#include "UI/BoundCheckBox.h"
#include "UI/BoundComboBox.h"
#include "UI/BoundEditableTextBox.h"
#include "UI/WeaponViewModel.h"
#include "UnrealExDNKUtils.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"


void UWeaponComponentBaseWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UWeaponComponentBaseWidget::NativeDestruct()
{
	ViewModel = nullptr;
	PropertyWidgets.Empty();

	Super::NativeDestruct();
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

	OnSetViewModel.Broadcast(InViewModel);
}

void UWeaponComponentBaseWidget::UpdateUIFromViewModel()
{
	if (ViewModel.IsValid() == false)
	{
		UE_DNK_LOG(LogTemp, Error, "ViewModel is invalid!");
		return;
	}

	PropertyWidgets.Empty();

	for (TFieldIterator<FProperty> PropIt(UWeaponViewModel::StaticClass()); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		FName PropertyName = Property->GetFName();

		// HorizontalBox row
		UHorizontalBox* RowBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

		// TextBlock for label
		UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Label->SetText(FText::FromName(PropertyName));

		// Right side: SizeBox -> ScaleBox -> Editable Widget
		USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		UScaleBox* ScaleBox = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass());

		UBoundWidgetInputBase* InputWidget = nullptr;

		if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
		{
			if (BoundCheckBoxClass != nullptr)
			{
				if (UBoundCheckBox* BoundCheckBox = WidgetTree->ConstructWidget<UBoundCheckBox>(BoundCheckBoxClass))
				{
					BoundCheckBox->BoundPropertyName = PropertyName;
					BoundCheckBox->WidgetInputReceiver = TScriptInterface<IWidgetInputReceiver>(this);

					void* ValuePtr = BoolProperty->ContainerPtrToValuePtr<void>(ViewModel.Get());
					bool bValue = BoolProperty->GetPropertyValue(ValuePtr);

					BoundCheckBox->CheckBox->SetIsChecked(bValue);
					InputWidget = BoundCheckBox;
				}
			}
			else
			{
				UE_DNK_LOG(LogTemp, Error, "BoundCheckBoxClass is INVALID!");
			}
		}
		else if (FIntProperty* IntProperty = CastField<FIntProperty>(Property))
		{
			if (BoundEditableTextBoxClass != nullptr)
			{
				if (UBoundEditableTextBox* BoundTextBox = WidgetTree->ConstructWidget<UBoundEditableTextBox>(BoundEditableTextBoxClass))
				{
					BoundTextBox->BoundPropertyName = PropertyName;
					BoundTextBox->WidgetInputReceiver = TScriptInterface<IWidgetInputReceiver>(this);

					void* ValuePtr = IntProperty->ContainerPtrToValuePtr<void>(ViewModel.Get());
					int32 Value = IntProperty->GetPropertyValue(ValuePtr);

					BoundTextBox->TextBox->SetText(FText::AsNumber(Value));
					InputWidget = BoundTextBox;
				}
			}
			else
			{
				UE_DNK_LOG(LogTemp, Error, "BoundEditableTextBoxClass is INVALID!");
			}
		}
		else if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
		{
			if (BoundEditableTextBoxClass != nullptr)
			{
				if (UBoundEditableTextBox* BoundTextBox = WidgetTree->ConstructWidget<UBoundEditableTextBox>(BoundEditableTextBoxClass))
				{
					BoundTextBox->BoundPropertyName = PropertyName;
					BoundTextBox->WidgetInputReceiver = TScriptInterface<IWidgetInputReceiver>(this);

					void* ValuePtr = FloatProperty->ContainerPtrToValuePtr<void>(ViewModel.Get());
					float Value = FloatProperty->GetPropertyValue(ValuePtr);

					BoundTextBox->TextBox->SetText(FText::AsNumber(Value));
					InputWidget = BoundTextBox;
				}
			}
		}
		else if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
		{
			if (BoundComboBoxClass != nullptr)
			{
				if (UBoundComboBox* BoundComboBoxWidget = WidgetTree->ConstructWidget<UBoundComboBox>(BoundComboBoxClass))
				{
					BoundComboBoxWidget->BoundPropertyName = PropertyName;
					BoundComboBoxWidget->WidgetInputReceiver = TScriptInterface<IWidgetInputReceiver>(this);

					UEnum* Enum = EnumProperty->GetEnum();
					if (Enum)
					{
						int32 NumEnums = Enum->NumEnums();
						for (int32 i = 0; i < NumEnums; ++i)
						{
							// Skip _MAX or hidden entries if needed
							if (!Enum->HasMetaData(TEXT("Hidden"), i))
							{
								FString EnumName = Enum->GetNameStringByIndex(i);
								BoundComboBoxWidget->ComboBox->AddOption(EnumName);
							}
						}
					}

					void* PropValuePtr = EnumProperty->ContainerPtrToValuePtr<void>(ViewModel.Get());
					int64 EnumIntValue = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(PropValuePtr);

					FString SelectedOption = Enum->GetNameStringByValue(EnumIntValue);
					BoundComboBoxWidget->ComboBox->SetSelectedOption(SelectedOption);
					InputWidget = BoundComboBoxWidget;
				}
			}
			else
			{
				UE_DNK_LOG(LogTemp, Error, "BoundComboBoxClass is INVALID!");
			}
		}
		else if (FClassProperty* ClassProperty = CastField<FClassProperty>(Property))
		{
			if (BoundComboBoxClass != nullptr)
			{
				if (UBoundComboBox* BoundComboBoxWidget = WidgetTree->ConstructWidget<UBoundComboBox>(BoundComboBoxClass))
				{
					BoundComboBoxWidget->BoundPropertyName = PropertyName;
					BoundComboBoxWidget->WidgetInputReceiver = TScriptInterface<IWidgetInputReceiver>(this);

					void* PropValuePtr = ClassProperty->ContainerPtrToValuePtr<void>(ViewModel.Get());
					UClass* SelectedClass = Cast<UClass>(ClassProperty->GetObjectPropertyValue(PropValuePtr));
					if (SelectedClass != nullptr)
					{
						TArray<UClass*> Subclasses;
						if (PropertyName.ToString().Contains("ProjectileClass"))
						{
							UUnrealExDNKUtils::GetAllSubclassesOf(AProjectileBase::StaticClass(), Subclasses);
						}

						for (UClass* Class : Subclasses)
						{
							FString ClassName = Class->GetName();
							BoundComboBoxWidget->ComboBox->AddOption(ClassName);
						}

						if (BoundComboBoxWidget->ComboBox->GetOptionCount() > 0)
						{
							BoundComboBoxWidget->ComboBox->SetSelectedOption(SelectedClass->GetName());
						}

						InputWidget = BoundComboBoxWidget;
					}
					else
					{
						UE_DNK_LOG(LogTemp, Error, "Can't convert property [%s] value to UClass", *PropertyName.ToString());
					}
				}
			}
			else
			{
				UE_DNK_LOG(LogTemp, Error, "BoundComboBoxClass is INVALID!");
			}
		}

		if (InputWidget)
		{
			PropertyWidgets.Add(PropertyName, InputWidget);

			ScaleBox->AddChild(InputWidget);
			SizeBox->AddChild(ScaleBox);

			RowBox->AddChildToHorizontalBox(Label);
			RowBox->AddChildToHorizontalBox(SizeBox);
			PropertiesVerticalBox->AddChildToVerticalBox(RowBox);
		}
	}
}

void UWeaponComponentBaseWidget::UpdateViewModelFromUI()
{
	if (ViewModel.IsValid() == false)
	{
		UE_DNK_LOG(LogTemp, Error, "ViewModel is invalid!");
		return;
	}

	for (TFieldIterator<FProperty> PropIt(UWeaponViewModel::StaticClass()); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		FName PropertyName = Property->GetFName();

		if (UBoundWidgetInputBase* WidgetPtr = PropertyWidgets.FindRef(PropertyName))
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(ViewModel.Get());

			if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
			{
				if (UBoundCheckBox* CheckBox = Cast<UBoundCheckBox>(WidgetPtr))
				{
					bool bIsChecked = CheckBox->CheckBox->IsChecked();
					BoolProperty->SetPropertyValue(ValuePtr, bIsChecked);
				}
			}
			else if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
			{
				if (UBoundComboBox* ComboBox = Cast<UBoundComboBox>(WidgetPtr))
				{
					FString SelectedItem = ComboBox->ComboBox->GetSelectedOption();
					UEnum* Enum = EnumProperty->GetEnum();
					int64 EnumValue = Enum->GetValueByNameString(SelectedItem);
					if (Enum->IsValidEnumValue(EnumValue))
					{
						EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(ValuePtr, EnumValue);
					}
				}
			}
			else if (FIntProperty* IntProperty = CastField<FIntProperty>(Property))
			{
				if (UBoundEditableTextBox* TextBox = Cast<UBoundEditableTextBox>(WidgetPtr))
				{
					const FString TextStr = TextBox->TextBox->GetText().ToString();
					int32 Value = FCString::Atoi(*TextStr);
					IntProperty->SetPropertyValue(ValuePtr, Value);
				}
			}
			else if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
			{
				if (UBoundEditableTextBox* TextBox = Cast<UBoundEditableTextBox>(WidgetPtr))
				{
					const FString TextStr = TextBox->TextBox->GetText().ToString();
					float Value = FCString::Atof(*TextStr);
					FloatProperty->SetPropertyValue(ValuePtr, Value);
				}
			}

			UE::FieldNotification::FFieldId FieldId = ViewModel->GetFieldNotificationDescriptor().GetField(ViewModel->GetClass(), PropertyName);
			ViewModel->BroadcastFieldValueChanged(FieldId);
		}
	}

	ViewModel->ApplyToCurrentWeapon();

	OnUpdateViewModelFromUI.Broadcast(ViewModel);
}

void UWeaponComponentBaseWidget::OnEnumSelectionChanged(FName PropertyName, const FString& SelectedItem)
{
	if (ViewModel.IsValid() == false)
	{
		UE_DNK_LOG(LogTemp, Error, "ViewModel is invalid!");
		return;
	}

	FProperty* Property = ViewModel->GetClass()->FindPropertyByName(PropertyName);
	if (!Property)
	{
		UE_DNK_LOG(LogTemp, Error, "Property %s not found on ViewModel!", *PropertyName.ToString());
		return;
	}

	if (FEnumProperty* EnumProp = CastField<FEnumProperty>(Property))
	{
		UEnum* Enum = EnumProp->GetEnum();
		int64 EnumValue = Enum->GetValueByNameString(SelectedItem);
		if (Enum->IsValidEnumValue(EnumValue))
		{
			void* PropValuePtr = EnumProp->ContainerPtrToValuePtr<void>(ViewModel.Get());
			EnumProp->GetUnderlyingProperty()->SetIntPropertyValue(PropValuePtr, EnumValue);

			UE::FieldNotification::FFieldId FieldId = ViewModel->GetFieldNotificationDescriptor().GetField(ViewModel->GetClass(), PropertyName);
			ViewModel->BroadcastFieldValueChanged(FieldId);
		}
	}
}

void UWeaponComponentBaseWidget::OnCheckStateChanged(FName PropertyName, bool bIsChecked)
{
	if (ViewModel.IsValid() == false)
	{
		UE_DNK_LOG(LogTemp, Error, "ViewModel is invalid!");
		return;
	}

	FProperty* Property = ViewModel->GetClass()->FindPropertyByName(PropertyName);
	if (!Property)
	{
		UE_DNK_LOG(LogTemp, Error, "Property %s not found on ViewModel!", *PropertyName.ToString());
		return;
	}

	if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
	{
		void* PropValuePtr = BoolProperty->ContainerPtrToValuePtr<void>(ViewModel.Get());
		BoolProperty->SetPropertyValue(PropValuePtr, bIsChecked);

		UE::FieldNotification::FFieldId FieldId = ViewModel->GetFieldNotificationDescriptor().GetField(ViewModel->GetClass(), PropertyName);
		ViewModel->BroadcastFieldValueChanged(FieldId);
	}
}

void UWeaponComponentBaseWidget::OnTextCommitted(FName PropertyName, const FText& NewText, ETextCommit::Type CommitMethod)
{
	if (!ViewModel.IsValid())
	{
		UE_DNK_LOG(LogTemp, Error, "ViewModel is invalid!");
		return;
	}

	FProperty* Property = ViewModel->GetClass()->FindPropertyByName(PropertyName);
	if (!Property)
	{
		UE_DNK_LOG(LogTemp, Error, "Property %s not found on ViewModel!", *PropertyName.ToString());
		return;
	}

	const FString TextStr = NewText.ToString();
	if (FIntProperty* IntProperty = CastField<FIntProperty>(Property))
	{
		int32 IntValue = FCString::Atoi(*TextStr);
		void* PropValuePtr = IntProperty->ContainerPtrToValuePtr<void>(ViewModel.Get());
		IntProperty->SetPropertyValue(PropValuePtr, IntValue);
	}
	else if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
	{
		float FloatValue = FCString::Atof(*TextStr);
		void* PropValuePtr = FloatProperty->ContainerPtrToValuePtr<void>(ViewModel.Get());
		FloatProperty->SetPropertyValue(PropValuePtr, FloatValue);
	}
	else
	{
		UE_DNK_LOG(LogTemp, Warning, "Property %s is not int or float", *PropertyName.ToString());
		return;
	}

	UE::FieldNotification::FFieldId FieldId = ViewModel->GetFieldNotificationDescriptor().GetField(ViewModel->GetClass(), PropertyName);
	ViewModel->BroadcastFieldValueChanged(FieldId);
}
