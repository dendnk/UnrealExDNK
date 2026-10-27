// Copyright 2024 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#include "UnrealExDNKUtils.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"


UWorld* UUnrealExDNKUtils::GetWorldSafe(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
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

bool UUnrealExDNKUtils::IsWithinRange(const float Value, const FVector2D& Range)
{
    return Value >= Range.GetMin() && Value <= Range.GetMax();
}

uint8 UUnrealExDNKUtils::CompressFloatToByte(float Value)
{
    return FMath::Clamp(FMath::RoundToInt((Value * 0.5f + 0.5f) * 255.f), 0, 255);
}

void UUnrealExDNKUtils::GetAllSubclassesOf(UClass* BaseClass, TArray<UClass*>& OutClasses)
{
    if (BaseClass == nullptr)
    {
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
