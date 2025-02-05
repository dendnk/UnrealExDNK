// Copyright 2024 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#include "UnrealExDNKUtils.h"

#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
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
#if WITH_EDITOR

    ISourceControlModule& SourceControlModule = FModuleManager::LoadModuleChecked<ISourceControlModule>("SourceControl");
    if (SourceControlModule.IsEnabled() && SourceControlModule.GetProvider().IsAvailable())
    {
        FString ProviderName = SourceControlModule.GetProvider().GetName().ToString();

        UE_LOG(LogTemp, Log, TEXT("Project is connected to Source Control: %s"), *ProviderName);

        if (ProviderName == TEXT("Git"))
        {
            FString GitPath = TEXT("git");
            FString RepoPath = FPaths::ProjectDir();
            FString Command = TEXT("rev-parse --short HEAD");
            FString Result;
            int32 ExitCode;

            FPlatformProcess::ExecProcess(*GitPath, *Command, &ExitCode, &Result, nullptr);
            if (ExitCode == 0)
            {
                return Result.TrimEnd();
            }
        }
        else if (ProviderName == TEXT("Perforce"))
        {
            UE_LOG(LogTemp, Log, TEXT("The project is using Perforce for source control."));
        }
        else if (ProviderName == TEXT("Plastic SCM"))
        {
            UE_LOG(LogTemp, Log, TEXT("The project is using Plastic SCM for source control."));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("The project is using an unknown source control system: %s"), *ProviderName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("The project is not connected to any source control."));
    }

#endif // WITH_EDITOR

    return TEXT("Unknown");
}

FVector UUnrealExDNKUtils::GenerateRandomDirectionXZ(const FVector2D& RangeRadians)
{
    float RandomAngle = FMath::RandRange(RangeRadians.GetMin(), RangeRadians.GetMax());
    return FVector(FMath::Cos(RandomAngle), 0.f, FMath::Sin(RandomAngle));
}

bool UUnrealExDNKUtils::IsWithinRange(const float Value, const FVector2D& Range)
{
    return Value >= Range.GetMin() && Value <= Range.GetMax();
}
