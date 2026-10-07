// Copyright 2024 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#include "UI/UserWidgetSpawnerComponent.h"
#include "Blueprint/UserWidget.h"
#include "IOwningActorInterface.h"
#include "UnrealExDNKUtils.h"


UUserWidgetSpawnerComponent::UUserWidgetSpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUserWidgetSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

    if (bShouldHaveUIWidget)
    {
        CreateAndAttachWidget();
    }
}

void UUserWidgetSpawnerComponent::CreateAndAttachWidget()
{
    if (IsValid(UIClass) == false)
    {
        return;
    }

    OwningPlayerController = UUnrealExDNKUtils::GetPlayerController(GetOwner());
    if (OwningPlayerController.IsValid() == false)
    {
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
