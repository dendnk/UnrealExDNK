// Fill out your copyright notice in the Description page of Project Settings.

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
};
