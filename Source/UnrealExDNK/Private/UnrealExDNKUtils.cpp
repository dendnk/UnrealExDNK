// Copyright 2024 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#include "UnrealExDNKUtils.h"

#if WITH_EDITOR
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#endif // WITH_EDITOR

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"


UWorld* UUnrealExDNKUtils::GetWorldSafe(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        UE_LOG(LogTemp, Error, TEXT("GetWorldSafe: WorldContextObject is null!"));
        return nullptr;
    }

    if (UWorld* World = WorldContextObject->GetWorld())
    {
        return World;
    }
    else if (WorldContextObject->GetOuter())
    {
        if (UWorld* OuterWorld = WorldContextObject->GetOuter()->GetWorld())
        {
            return OuterWorld;
		}
    }
    else if (GEngine)
    {
        if (UWorld* EngineWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
        {
            return EngineWorld;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("GetWorldSafe: Cannot get World from WorldContextObject!"));
    return nullptr;
}

APlayerController* UUnrealExDNKUtils::GetPlayerController(const UObject* Object, int32 PlayerIndex)
{
    if (IsValid(Object) == false)
    {
        return nullptr;
    }

    if (const APlayerController* PlayerController = Cast<APlayerController>(Object))
    {
        return const_cast<APlayerController*>(PlayerController);
    }
    else if (const APawn* Pawn = Cast<APawn>(Object))
    {
        return GetPlayerController(Pawn->GetController());
    }
    else if (const UWorld* World = Cast<UWorld>(Object))
    {
        return UGameplayStatics::GetPlayerController(World, PlayerIndex);
    }

	return nullptr;
}

#if WITH_EDITOR
FString UUnrealExDNKUtils::GetGitCommitHash()
{
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

    return TEXT("Unknown");
}

#endif // WITH_EDITOR

FVector UUnrealExDNKUtils::GenerateRandomDirectionXZ(const FVector2D& RangeRadians)
{
    float RandomAngle = FMath::RandRange(RangeRadians.GetMin(), RangeRadians.GetMax());
    return FVector(FMath::Cos(RandomAngle), 0.f, FMath::Sin(RandomAngle));
}

bool UUnrealExDNKUtils::IsWithinRange(const float Value, const FVector2D& Range)
{
    return Value >= Range.GetMin() && Value <= Range.GetMax();
}

int32 UUnrealExDNKUtils::SumIntArray(const TArray<int32>& Array)
{
    return SumArray_Internal<int32>(Array);
}

float UUnrealExDNKUtils::SumFloatArray(const TArray<float>& Array)
{
    return SumArray_Internal<float>(Array);
}

uint8 UUnrealExDNKUtils::SumByteArray(const TArray<uint8>& Array)
{
    return SumArray_Internal<uint8>(Array);
}

uint8 UUnrealExDNKUtils::CompressFloatToByte(float Value)
{
    return FMath::Clamp(FMath::RoundToInt((Value * 0.5f + 0.5f) * 255.f), 0, 255);
}

float UUnrealExDNKUtils::DecompressByteToFloat(uint8 ByteValue)
{
    return ((float)ByteValue / 255.f) * 2.f - 1.f;
}

FVector UUnrealExDNKUtils::TryGetMeshScaleFromCDO(TSubclassOf<AActor> ActorClass)
{
    if (IsValid(ActorClass) == false)
    {
        return FVector::OneVector;
    }

    const AActor* DefaultObject = ActorClass->GetDefaultObject<AActor>();
    if (IsValid(DefaultObject) == false)
    {
        return FVector::OneVector;
    }

    if (const UStaticMeshComponent* MeshComponent = DefaultObject->FindComponentByClass<UStaticMeshComponent>())
    {
        return MeshComponent->GetRelativeScale3D();
    }

    return FVector::OneVector;
}

void UUnrealExDNKUtils::GetAllSubclassesOf(UClass* BaseClass, TArray<UClass*>& OutClasses)
{
    if (BaseClass == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("BaseClass is nullptr!"));
        return;
    }

    for (TObjectIterator<UClass> It; It; ++It)
    {
        UClass* Candidate = *It;

        if (Candidate->IsChildOf(BaseClass) == false ||
            Candidate->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_HideDropDown))
        {
            continue;
        }

        OutClasses.Add(Candidate);
    }
}
