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

/** Log stack */
#define DNK_LOG_STACK_TRACE() \
{ \
	ANSICHAR StackBuffer[4096]; \
	FPlatformStackWalk::StackWalkAndDump(StackBuffer, sizeof(StackBuffer), 0, nullptr); \
	UE_LOG(LogTemp, Warning, TEXT("Stack:\n%s"), ANSI_TO_TCHAR(StackBuffer)); \
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

template <typename T>
T SumArray_Internal(const TArray<T>& Array)
{
	T Sum = 0;
	for (const T& Value : Array)
	{
		Sum += Value;
	}
	return Sum;
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
	UFUNCTION(BlueprintCallable, Category = "Player Controller", meta = (WorldContext = "WorldContextObject"))
	static APlayerController* GetPlayerController(UObject* WorldContextObject);

	/**
	 * Returns current git commit hash
	 *
	 * @return Git commit hash or "unknown" in failed case
	 */
	UFUNCTION(BlueprintCallable, Category = "Version Control Systems")
	static FString GetGitCommitHash();

	/**
	 * Generate random Vector Direction in the XZ plane in the specified range in radians
	 *
	 * @return Direction in XZ plane
	 */
	UFUNCTION(BlueprintCallable, Category = "Math")
	static FVector GenerateRandomDirectionXZ(const FVector2D& RangeRadians);

	/**
	 * Checking if a value lies within a range defined by FVector2D
	 *
	 * @return true if Value lies within a range
	 */
	UFUNCTION(BlueprintCallable, Category = "Math")
	static bool IsWithinRange(const float Value, const FVector2D& Range);

	/**
	 * This is equivalent to FLT_MAX and can be used when initializing variables
	 * for comparison in min/max calculations or when representing a very large number.
	 *
	 * @return The largest possible float value (FLT_MAX).
	 */
	UFUNCTION(BlueprintPure, Category = "Math")
	static float GetMaxFloat() { return FLT_MAX; }

	/**
	 * Calculates the sum of all elements in an integer array.
	 *
	 * This utility function returns the total of all int32 values contained in the provided array.
	 * Internally uses a templated helper function to support type reuse.
	 *
	 * @param Array The array of int32 values to be summed.
	 * @return The sum of all elements in the array. Returns 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Array")
	static int32 SumIntArray(const TArray<int32>& Array);

	/**
	 * Calculates the sum of all elements in an float array.
	 *
	 * This utility function returns the total of all int32 values contained in the provided array.
	 * Internally uses a templated helper function to support type reuse.
	 *
	 * @param Array The array of int32 values to be summed.
	 * @return The sum of all elements in the array. Returns 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Array")
	static float SumFloatArray(const TArray<float>& Array);

	/**
	 * Calculates the sum of all elements in a byte array.
	 *
	 * This utility function returns the total of all int32 values contained in the provided array.
	 * Internally uses a templated helper function to support type reuse.
	 *
	 * @param Array The array of int32 values to be summed.
	 * @return The sum of all elements in the array. Returns 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Array")
	static uint8 SumByteArray(const TArray<uint8>& Array);

	/**
	 * Compresses a float in the range [-1.0, 1.0] to an 8-bit unsigned integer [0, 255].
	 *
	 * This is useful for network serialization or GPU packing, where a normalized float
	 * needs to be stored in a single byte. Values outside the range are clamped.
	 *
	 * @param Value A float value expected to be in the range [-1.0, 1.0].
	 * @return An 8-bit unsigned integer representing the compressed float in [0, 255].
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Compressing")
	static uint8 CompressFloatToByte(float Value);

	/**
	 * Decompresses an 8-bit unsigned integer [0, 255] back into a float in the range [-1.0, 1.0].
	 *
	 * This is the inverse of CompressFloatToByte. It maps a byte value back to its
	 * original normalized float representation, useful when decoding values from network
	 * packets, GPU buffers, or packed data formats.
	 *
	 * @param ByteValue An 8-bit unsigned integer representing the compressed float.
	 * @return The decompressed float value in the range [-1.0, 1.0].
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Compressing")
	static float DecompressByteToFloat(uint8 ByteValue);

	/**
	 * Attempts to retrieve the relative scale of the first UStaticMeshComponent
	 * found in the Class Default Object (CDO) of the given Actor class.
	 *
	 * This is useful when you want to know how a mesh is scaled in a Blueprint-derived
	 * class before actually spawning an instance of it.
	 *
	 * @param ActorClass The actor class to inspect (can be a Blueprint or native class).
	 * @return The relative scale (X, Y, Z) of the mesh component if found; otherwise, FVector::OneVector.
	 */
	UFUNCTION(BlueprintPure, Category = "CDO")
	static FVector TryGetMeshScaleFromCDO(TSubclassOf<AActor> ActorClass);

	UFUNCTION(BlueprintCallable, Category = "Class Utils")
	static void GetAllSubclassesOf(UClass* BaseClass, TArray<UClass*>& OutClasses);
};
