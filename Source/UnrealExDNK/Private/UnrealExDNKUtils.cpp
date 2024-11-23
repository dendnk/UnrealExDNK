// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealExDNKUtils.h"
#include "HAL/PlatformProcess.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Misc/Paths.h"


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

FString UUnrealExDNKUtils::GetGitCommitHash()
{
    FString GitPath = TEXT("git");
    FString RepoPath = FPaths::ProjectDir();
    FString Command = TEXT("rev-parse --short HEAD");
    FString Result;
    int32 ExitCode;

    FPlatformProcess::ExecProcess(*GitPath, *Command, &ExitCode, &Result, nullptr);
    return (ExitCode == 0) ? Result.TrimEnd() : TEXT("Unknown");
}
