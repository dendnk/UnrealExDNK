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

namespace ArrayUtils
{
	/**
	 * Removes all invalid (expired) weak pointers from the given array.
	 *
	 * Example:
	 *     WeakArrayUtils::CleanArray(MyWeakActors);
	 */
	template<typename TObject>
	void CleanArray(TArray<TWeakObjectPtr<TObject>>& Array)
	{
		Array.RemoveAll([](const TWeakObjectPtr<TObject>& Ptr)
			{
				return !Ptr.IsValid();
			});
	}

}

namespace ActorComponents
{
	template <typename T>
	TArray<T*> GetComponentsByCondition(const AActor* Owner, TFunctionRef<bool(const T*)> Predicate)
	{
		TArray<T*> Result;
		if (!Owner) return Result;

		TInlineComponentArray<T*> Components(Owner);
		for (T* Comp : Components)
		{
			if (Comp && Predicate(Comp))
			{
				Result.Add(Comp);
			}
		}
		return Result;
	}
}


/**
 *
 */
UCLASS()
class UNREALEXDNK_API UUnrealExDNKUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// Helper function for safe world access
	static UWorld* GetWorldSafe(const UObject* WorldContextObject);

	/**
	 * Returns the owning Player Controller.
	 *
	 * @param Object Object context from which to derive the player controller.
	 * @return The owning Player Controller or nullptr if not found.
	 */
	UFUNCTION(BlueprintCallable, Category = "Player Controller")
	static APlayerController* GetPlayerController(const UObject* Object, int32 PlayerIndex = 0);

	/**
	 * Checking if a value lies within a range defined by FVector2D
	 *
	 * @return true if Value lies within a range
	 */
	UFUNCTION(BlueprintCallable, Category = "Math")
	static bool IsWithinRange(const float Value, const FVector2D& Range);


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

	UFUNCTION(BlueprintCallable, Category = "Class Utils")
	static void GetAllSubclassesOf(UClass* BaseClass, TArray<UClass*>& OutClasses);

	template<typename TEnum>
	static FString GetEnumAsString(TEnum InEnum)
	{
		static_assert(TIsEnum<TEnum>::Value, "TEnum must be an enum type.");

		if (const UEnum* EnumPtr = StaticEnum<TEnum>())
		{
			return EnumPtr->GetNameStringByValue(static_cast<int64>(InEnum));
		}

		return TEXT("Invalid");
	}

	template <typename BaseType, typename DerivedType>
	static TArray<BaseType*> ConvertArrayToBase(const TArray<DerivedType*>& InArray)
	{
		static_assert(TIsDerivedFrom<DerivedType, BaseType>::IsDerived,
			"ConvertArray: DerivedType must inherit from BaseType");

		TArray<BaseType*> OutArray;
		OutArray.Reserve(InArray.Num());

		for (DerivedType* Item : InArray)
		{
			OutArray.Add(Item); // implicit upcast
		}
		return OutArray;
	}

	template <typename ToType, typename FromType>
	static TArray<ToType*> ConvertArray(const TArray<FromType*>& InArray)
	{
		static_assert(TIsDerivedFrom<ToType, FromType>::IsDerived,
			"ConvertArray: ToType must inherit from FromType");

		TArray<ToType*> OutArray;
		OutArray.Reserve(InArray.Num());

		for (FromType* Item : InArray)
		{
			if (ToType* CastedItem = Cast<ToType>(Item))
			{
				OutArray.Add(CastedItem);
			}
		}
		return OutArray;
	}

	template<typename Type>
	static bool IsSame(const TArray<Type>& InArrayA, const TArray<Type>& InArrayB)
	{
		bool bIsSameNum = InArrayA.Num() == InArrayB.Num();
		if (bIsSameNum == false)
		{
			return false;
		}

		for (int32 i = 0; i < InArrayA.Num(); ++i)
		{
			if (InArrayA[i] != InArrayB[i])
			{
				return false;
			}
		}

		return true;
	}
};
