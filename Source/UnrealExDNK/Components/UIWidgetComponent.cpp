// Copyright 2024 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#include "UIWidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "UnrealExDNKUtils.h"


UUIWidgetComponent::UUIWidgetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	CreateAndAttachWidget();
}

void UUIWidgetComponent::CreateAndAttachWidget()
{
    if (!IsValid(UIClass))
    {
        UE_LOG(LogTemp, Warning, TEXT("UIClass is not specified for component: %s"), *GetName());
        return;
    }

    OwningPlayerController = UUnrealExDNKUtils::GetPlayerController(GetOwner());
    if (!IsValid(OwningPlayerController))
    {
        UE_LOG(LogTemp, Warning, TEXT("OwningController is not valid for component: %s"), *GetName());
        return;
    }

    WidgetInstance = CreateWidget<UUserWidget>(OwningPlayerController, UIClass);
    if (IsValid(WidgetInstance))
    {
        WidgetInstance->AddToViewport();
    }
}
