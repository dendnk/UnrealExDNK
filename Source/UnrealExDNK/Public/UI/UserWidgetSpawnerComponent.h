// Copyright 2024 [UnrealExDNK: Denis Kruchok]. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "UserWidgetSpawnerComponent.generated.h"

class APlayerController;
class UUserWidget;

UCLASS(Abstract, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALEXDNK_API UUserWidgetSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UUserWidgetSpawnerComponent();

    UPROPERTY(EditDefaultsOnly, Category = UserWidgetSpawnerComponent)
    bool bShouldHaveUIWidget = true;

protected:
	virtual void BeginPlay() override;
    virtual void CreateAndAttachWidget();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UserWidgetSpawnerComponent)
    TSubclassOf<UUserWidget> UIClass;

    UPROPERTY(BlueprintReadOnly, Category = UserWidgetSpawnerComponent)
    TObjectPtr<UUserWidget> WidgetInstance;

    UPROPERTY(BlueprintReadOnly, Category = UserWidgetSpawnerComponent)
    TWeakObjectPtr<APlayerController> OwningPlayerController;
};
