// Copyright 2024 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/World.h"
#include "UnrealExDNKUtils.generated.h"

class APlayerController;

#define NETMODE_WORLD \
	( (( GetWorld() == nullptr )) ? TEXT("") \
	: (( GEngine->GetNetMode( GetWorld() ) == NM_Client)) ? TEXT("Client") \
	: (( GEngine->GetNetMode( GetWorld() ) == NM_ListenServer)) ? TEXT("ListenServer") \
	: (( GEngine->GetNetMode( GetWorld() ) == NM_DedicatedServer)) ? TEXT("DedicatedServer") \
	: TEXT("Standalone") \
	)

/** Use only for UObject based classes */
#define UE_DNK_LOG(Category, Verbosity, Format, ...) \
{ \
	if (const UObject* AsUObject = Cast<UObject>(this)) \
	{ \
		UE_LOG(Category, Verbosity, TEXT("[%s]-[%s]-[%s] : %s"), \
			* GetNameSafe(AsUObject), NETMODE_WORLD, ANSI_TO_TCHAR(__FUNCTION__), \
			* FString::Printf(TEXT(Format), ##__VA_ARGS__)); \
	} \
}

template <typename TEnum>
FString EnumToString(TEnum EnumValue)
{
	const UEnum* EnumPtr = StaticEnum<TEnum>();
	if (!EnumPtr)
	{
		return TEXT("Invalid Enum");
	}

	return EnumPtr->GetNameByValue(static_cast<int64>(EnumValue)).ToString();
}

UENUM(BlueprintType)
enum class EParameterType : uint8
{
	Undefined,
	Boolean,
	Integer,
	Float,
	String,
	Name,
	Vector,
	Rotator
};

USTRUCT(BlueprintType)
struct FStructElement
{
	GENERATED_BODY()

	FStructElement() {}

	FStructElement(const FString& InName, const EParameterType& InType, const FString& InValue)
		:	ParameterName(InName)
		,	ParameterType(InType)
		,	ParameterValue(InValue)
	{}

	UPROPERTY(BlueprintReadWrite)
	FString ParameterName;

	UPROPERTY(BlueprintReadWrite)
	EParameterType ParameterType;

	UPROPERTY(BlueprintReadWrite)
	FString ParameterValue;
};

template<typename T>
constexpr bool IsUStruct()
{
	if constexpr (TIsDerivedFrom<T, FTableRowBase>::IsDerived)
	{
		return true;
	}
	else
	{
		return T::StaticStruct() != nullptr;
	}
}

template<typename T>
TArray<FStructElement> ConvertStructToArrayOfElements(const T& StructInstance)
{
	//static_assert(IsUStruct(StructInstance) == false, "Struct must be a USTRUCT");

	TArray<FStructElement> Results;

	const UStruct* Struct = T::StaticStruct();
	for (TFieldIterator<FProperty> It(Struct); It; ++It)
	{
		const FProperty* Property = *It;
		const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(&StructInstance);

		FStructElement StructElement;
		StructElement.ParameterName = *Property->GetName();

		if (const FBoolProperty* BoolProp = CastField<const FBoolProperty>(Property))
		{
			StructElement.ParameterType = EParameterType::Boolean;
			bool bValue = BoolProp->GetPropertyValue(ValuePtr);
			StructElement.ParameterValue = bValue ? TEXT("true") : TEXT("false");
		}
		else if (const FFloatProperty* FloatProp = CastField<const FFloatProperty>(Property))
		{
			StructElement.ParameterType = EParameterType::Float;
			float Value = FloatProp->GetPropertyValue(ValuePtr);
			StructElement.ParameterValue = FString::SanitizeFloat(Value);
		}
		else if (const FIntProperty* IntProp = CastField<const FIntProperty>(Property))
		{
			StructElement.ParameterType = EParameterType::Integer;
			int32 Value = IntProp->GetPropertyValue(ValuePtr);
			StructElement.ParameterValue = FString::FromInt(Value);
		}
		else if (const FStrProperty* StrProp = CastField<const FStrProperty>(Property))
		{
			StructElement.ParameterType = EParameterType::String;
			StructElement.ParameterValue = StrProp->GetPropertyValue(ValuePtr);
		}
		else if (const FNameProperty* NameProp = CastField<const FNameProperty>(Property))
		{
			StructElement.ParameterType = EParameterType::Name;
			FName Value = NameProp->GetPropertyValue(ValuePtr);
			StructElement.ParameterValue = Value.ToString();
		}
		else
		{
			StructElement.ParameterType = EParameterType::Undefined;
			Property->ExportTextItem_Direct(StructElement.ParameterValue, ValuePtr, nullptr, nullptr, PPF_None);
		}


		Results.Emplace(StructElement);


		UE_LOG(LogTemp, Log, TEXT(	"StructElement:"
									"Name : %s"
									"Value : %s"
									"Type : %s"),
			*StructElement.ParameterName,
			*StructElement.ParameterValue,
			*EnumToString<EParameterType>(StructElement.ParameterType))
	}

	return Results;
}

template<typename T>
bool ApplyStructElementToStruct(T& StructInstance, const FStructElement& Element)
{
	//static_assert(TIsDerivedFrom<T, FTableRowBase>::IsDerived || TStructOpsTypeTraits<T>::WithSerializer, "Struct must be a USTRUCT");

	const UStruct* Struct = T::StaticStruct();
	FProperty* Property = Struct->FindPropertyByName(*Element.ParameterName);

	if (!Property)
	{
		UE_LOG(LogTemp, Warning, TEXT("Property '%s' not found in struct."), *Element.ParameterName);
		return false;
	}

	void* ValuePtr = Property->ContainerPtrToValuePtr<void>(&StructInstance);

	switch (Element.ParameterType)
	{
	case EParameterType::Boolean:
		if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
		{
			BoolProp->SetPropertyValue(ValuePtr, Element.ParameterValue.ToBool());
			return true;
		}
		break;

	case EParameterType::Float:
		if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
		{
			FloatProp->SetPropertyValue(ValuePtr, FCString::Atof(*Element.ParameterValue));
			return true;
		}
		break;

	case EParameterType::Integer:
		if (FIntProperty* IntProp = CastField<FIntProperty>(Property))
		{
			IntProp->SetPropertyValue(ValuePtr, FCString::Atoi(*Element.ParameterValue));
			return true;
		}
		break;

	case EParameterType::String:
		if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
		{
			StrProp->SetPropertyValue(ValuePtr, Element.ParameterValue);
			return true;
		}
		break;

	case EParameterType::Name:
		if (FNameProperty* NameProp = CastField<FNameProperty>(Property))
		{
			NameProp->SetPropertyValue(ValuePtr, FName(*Element.ParameterValue));
			return true;
		}
		break;

	default:
		break;
	}

	UE_LOG(LogTemp, Warning, TEXT("Failed to apply value to property '%s'. Type mismatch or unsupported type."),
		*Element.ParameterName);
	return false;

}

/**
 * 
 */
UCLASS()
class UNREALEXDNK_API UUnrealExDNKUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/**
	 * Returns the owning Player Controller.
	 *
	 * @param WorldContextObject Object context from which to derive the player controller.
	 * @return The owning Player Controller or nullptr if not found.
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealExDNK Utils", meta = (WorldContext = "WorldContextObject"))
	static APlayerController* GetPlayerController(UObject* WorldContextObject);

	/**
	 * Returns current git commit hash
	 *
	 * @return Git commit hash or "unknown" in failed case 
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealExDNK Utils")
	static FString GetGitCommitHash();

	/**
	 * Generate random Vector Direction in the XZ plane in the specified range in radians
	 *
	 * @return Direction in XZ plane
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealExDNK Utils")
	static FVector GenerateRandomDirectionXZ(const FVector2D& RangeRadians);

	/**
	 * Checking if a value lies within a range defined by FVector2D
	 *
	 * @return true if Value lies within a range
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealExDNK Utils")
	static bool IsWithinRange(const float Value, const FVector2D& Range);

	/**
	 * This is equivalent to FLT_MAX and can be used when initializing variables
	 * for comparison in min/max calculations or when representing a very large number.
	 *
	 * @return The largest possible float value (FLT_MAX).
	 */
	UFUNCTION(BlueprintPure, Category = "Math")
	static float GetMaxFloat() { return FLT_MAX; }
};
