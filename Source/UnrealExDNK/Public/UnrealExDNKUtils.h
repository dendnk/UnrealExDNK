// Copyright 2024 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UnrealExDNKUtils.generated.h"

class APlayerController;

#define NETMODE_WORLD \
	( (( GetWorld() == nullptr )) ? TEXT("") \
	: (( GEngine->GetNetMode( GetWorld() ) == NM_Client)) ? TEXT("Client") \
	: (( GEngine->GetNetMode( GetWorld() ) == NM_ListenServer)) ? TEXT("ListenServer") \
	: (( GEngine->GetNetMode( GetWorld() ) == NM_DedicatedServer)) ? TEXT("DedicatedServer") \
	: TEXT("Standalone") \
	)

#define UE_DNK_LOG(Category, Verbosity, Format, ...) \
{ \
	UE_LOG(Category, Verbosity, TEXT("[%s]-[%s]-[%s] : %s"), \
			*GetNameSafe(this), NETMODE_WORLD, TEXT(__FUNCTION__), \
			*FString::Printf(TEXT(Format), ##__VA_ARGS__)); \
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
	* Generate random Vector Direction in the XZ plane
	*
	* @return Direction in XZ plane
	*/
	UFUNCTION(BlueprintCallable, Category = "UnrealExDNK Utils")
	static FVector GenerateRandomDirectionXZ();

	/**
	* Checking if a value lies within a range defined by FVector2D
	*
	* @return true if Value lies within a range
	*/
	UFUNCTION(BlueprintCallable, Category = "UnrealExDNK Utils")
	static bool IsWithinRange(const float Value, const FVector2D& Range);
};
