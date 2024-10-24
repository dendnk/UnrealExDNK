// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "UnrealExDNKUtils.h"


APlayerController* UUnrealExDNKUtils::GetPlayerController(UObject* WorldContextObject)
{
    if (WorldContextObject == nullptr)
    {
        return nullptr;
    }

    if (APlayerController* PlayerController = Cast<APlayerController>(WorldContextObject))
    {
        return PlayerController;
    }

    if (APawn* Pawn = Cast<APawn>(WorldContextObject))
    {
        return GetPlayerController(Pawn->GetController());
    }

	return nullptr;
}