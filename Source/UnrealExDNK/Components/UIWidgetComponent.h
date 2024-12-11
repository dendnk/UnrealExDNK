// Copyright 2024 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "UIWidgetComponent.generated.h"

class APlayerController;
class UUserWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALEXDNK_API UUIWidgetComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UUIWidgetComponent();

protected:
	virtual void BeginPlay() override;
    virtual void CreateAndAttachWidget();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> UIClass;

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    TObjectPtr<UUserWidget> WidgetInstance;

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<APlayerController> OwningPlayerController;
};
