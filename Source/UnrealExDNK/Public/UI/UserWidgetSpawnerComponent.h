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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = UserWidgetSpawnerComponent)
    bool bShouldHaveUIWidget = true;

    UFUNCTION(BlueprintCallable)
    TSubclassOf<UUserWidget> GetUIClass() const { return UIClass; };

    UFUNCTION(BlueprintCallable)
    void SetUIClass(TSubclassOf<UUserWidget> NewUIClass) { UIClass = NewUIClass; };

protected:
	virtual void BeginPlay() override;
    virtual void CreateAndAttachWidget();


    UPROPERTY(BlueprintReadOnly, Category = UserWidgetSpawnerComponent)
    TObjectPtr<UUserWidget> WidgetInstance;

    UPROPERTY(BlueprintReadOnly, Category = UserWidgetSpawnerComponent)
    TWeakObjectPtr<APlayerController> OwningPlayerController;

private:
    UPROPERTY(EditAnywhere, Category = UserWidgetSpawnerComponent)
    TSubclassOf<UUserWidget> UIClass;
};
