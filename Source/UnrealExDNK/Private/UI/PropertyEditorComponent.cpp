// Copyright 2025 [UnrealExDNK: Denis Kruchok]. All rights reserved.


#include "UI/PropertyEditorComponent.h"
#include "UObject/UnrealType.h"


TArray<FPropertyInfo> UPropertyEditorComponent::GetEditableProperties() const
{
    TArray<FPropertyInfo> ResultProperties;

    if (TObjectPtr<AActor> Actor = GetOwner())
    {
        if (TObjectPtr<UClass> ActorClass = Actor->GetClass())
        {
            for (TFieldIterator<FProperty> PropIt(ActorClass); PropIt; ++PropIt)
            {
                FProperty* Property = *PropIt;
                if (Property->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible))
                {
                    FPropertyInfo PropertyData;
                    PropertyData.Name = Property->GetName();
                    PropertyData.Type = Property->GetCPPType();
                    void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Actor);

                    if (FIntProperty* IntProp = CastField<FIntProperty>(Property))
                    {
                        int32 Value = IntProp->GetPropertyValue(ValuePtr);
                        PropertyData.Value = FString::FromInt(Value);
                    }
                    else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
                    {
                        float Value = FloatProp->GetPropertyValue(ValuePtr);
                        PropertyData.Value = FString::SanitizeFloat(Value);
                    }
                    else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
                    {
                        bool bValue = BoolProp->GetPropertyValue(ValuePtr);
                        PropertyData.Value = bValue ? TEXT("true") : TEXT("false");
                    }
                    else if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
                    {
                        FString Value = StrProp->GetPropertyValue(ValuePtr);
                        PropertyData.Value = Value;
                    }
                    else if (FNameProperty* NameProp = CastField<FNameProperty>(Property))
                    {
                        FName Value = NameProp->GetPropertyValue(ValuePtr);
                        PropertyData.Value = Value.ToString();
                    }
                    else if (FObjectProperty* ObjectProp = CastField<FObjectProperty>(Property))
                    {
                        UObject* Obj = ObjectProp->GetObjectPropertyValue(ValuePtr);
                        PropertyData.Value = Obj ? Obj->GetName() : TEXT("null");
                    }
                    else if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
                    {
                        void* StructPtr = StructProp->ContainerPtrToValuePtr<void>(Actor);
                        FString StructName = StructProp->Struct->GetName();

                        if (StructProp->Struct == TBaseStructure<FVector>::Get())
                        {
                            FVector Vec = *static_cast<FVector*>(StructPtr);
                            PropertyData.Value = Vec.ToString();  // "X= Y= Z="
                        }
                        else if (StructProp->Struct == TBaseStructure<FRotator>::Get())
                        {
                            FRotator Rot = *static_cast<FRotator*>(StructPtr);
                            PropertyData.Value = Rot.ToString();  // "P= Y= R="
                        }
                        else
                        {
                            PropertyData.Value = FString::Printf(TEXT("<Struct: %s>"), *StructName);
                        }
                    }
                    else
                    {
                        PropertyData.Value = TEXT("<Unsupported Type>");
                        continue;
                    }

                    PropertyData.Log();
                    ResultProperties.Emplace(PropertyData);
                }
            }
        }
    }

    return ResultProperties;
}

bool UPropertyEditorComponent::ApplyPropertyValue(UObject* TargetObject, const FString& PropertyName, const FString& NewValueAsString)
{
    if (IsValid(TargetObject) == false)
    {
        return false;
    }
    if (IsValid(TargetObject->GetClass()) == false)
    {
        return false;
    }

    FProperty* Property = TargetObject->GetClass()->FindPropertyByName(*PropertyName);
    if (IsPropertyEditable(Property) == false)
    {
        return false;
    }

    void* ValuePtr = Property->ContainerPtrToValuePtr<void>(TargetObject);

    if (FIntProperty* IntProp = CastField<FIntProperty>(Property))
    {
        int32 IntValue = FCString::Atoi(*NewValueAsString);
        IntProp->SetPropertyValue(ValuePtr, IntValue);
        return true;
    }
    else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
    {
        float FloatValue = FCString::Atof(*NewValueAsString);
        FloatProp->SetPropertyValue(ValuePtr, FloatValue);
        return true;
    }
    else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
    {
        bool bValue = (NewValueAsString.ToLower() == "true" || NewValueAsString == "1");
        BoolProp->SetPropertyValue(ValuePtr, bValue);
        return true;
    }
    else if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
    {
        StrProp->SetPropertyValue(ValuePtr, NewValueAsString);
        return true;
    }
    else if (FNameProperty* NameProp = CastField<FNameProperty>(Property))
    {
        NameProp->SetPropertyValue(ValuePtr, FName(*NewValueAsString));
        return true;
    }
    else if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
    {
        if (StructProp->Struct == TBaseStructure<FVector>::Get())
        {
            FVector Vec;
            if (Vec.InitFromString(NewValueAsString))
            {
                *static_cast<FVector*>(ValuePtr) = Vec;
                return true;
            }
        }
        else if (StructProp->Struct == TBaseStructure<FRotator>::Get())
        {
            FRotator Rot;
            if (Rot.InitFromString(NewValueAsString))
            {
                *static_cast<FRotator*>(ValuePtr) = Rot;
                return true;
            }
        }
    }

    return false; // Unsupported type or parsing failed
}

bool UPropertyEditorComponent::IsPropertyEditable(FProperty* Property)
{
    return  Property != nullptr &&
            (Property->HasAnyPropertyFlags(CPF_EditConst | CPF_DisableEditOnInstance) == false) &&
            (Property->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible) == true);
}
