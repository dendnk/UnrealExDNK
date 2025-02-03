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

    UPROPERTY(EditDefaultsOnly, Category = UIWidgetComponent)
    bool bShouldHaveUIWidget = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UIWidgetComponent)
    TSubclassOf<UUserWidget> UIClass;

    UPROPERTY(BlueprintReadOnly, Category = UIWidgetComponent)
    TObjectPtr<UUserWidget> WidgetInstance;

    UPROPERTY(BlueprintReadOnly, Category = UIWidgetComponent)
    TObjectPtr<APlayerController> OwningPlayerController;
};
