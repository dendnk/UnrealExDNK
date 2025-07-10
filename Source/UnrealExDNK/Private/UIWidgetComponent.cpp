// Copyright 2024 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#include "UIWidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "OwningActorInterface.h"
#include "UnrealExDNKUtils.h"


UUIWidgetComponent::UUIWidgetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

    if (bShouldHaveUIWidget)
    {
        CreateAndAttachWidget();
    }
}

void UUIWidgetComponent::CreateAndAttachWidget()
{
    FString OwnerName = GetOwner() != nullptr ? *GetOwner()->GetName() : TEXT("Owner is nullptr!");
    if (IsValid(UIClass) == false)
    {
        UE_DNK_LOG(LogTemp, Warning, "UIClass is not specified for component [%s] | Owner [%s]. If it was done intentionally, set bShouldHaveUIWidget to false!"
            , *GetName()
            , *OwnerName);
        return;
    }

    OwningPlayerController = UUnrealExDNKUtils::GetPlayerController(GetOwner());
    if (OwningPlayerController.IsValid() == false)
    {
        UE_DNK_LOG(LogTemp, Warning, "OwningController is not valid for component [%s] | Owner [%s]", *GetName(), *OwnerName);
        return;
    }

    WidgetInstance = CreateWidget<UUserWidget>(OwningPlayerController.Get(), UIClass);
    if (IsValid(WidgetInstance))
    {
        if (IOwningActorInterface* OwningActorInterface = Cast<IOwningActorInterface>(WidgetInstance))
        {
            OwningActorInterface->Execute_SetOwningActor(WidgetInstance, GetOwner());
        }

        WidgetInstance->AddToViewport();
    }
}
