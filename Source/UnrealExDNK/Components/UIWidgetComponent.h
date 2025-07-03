// Copyright 2024 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "UIWidgetComponent.generated.h"

class APlayerController;
class UUserWidget;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALEXDNK_API UUIWidgetComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UUIWidgetComponent();

    UPROPERTY(EditDefaultsOnly, Category = UIWidgetComponent)
    bool bShouldHaveUIWidget = true;

protected:
	virtual void BeginPlay() override;
    virtual void CreateAndAttachWidget();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UIWidgetComponent)
    TSubclassOf<UUserWidget> UIClass;

    UPROPERTY(BlueprintReadOnly, Category = UIWidgetComponent)
    TObjectPtr<UUserWidget> WidgetInstance;

    UPROPERTY(BlueprintReadOnly, Category = UIWidgetComponent)
    TWeakObjectPtr<APlayerController> OwningPlayerController;
};
