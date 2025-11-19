// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractInterface.h"
#include "InteractiveActorBase.generated.h"


UCLASS()
class HYPERSHOOTER_API AInteractiveActorBase : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractiveActorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

/*
	Interactive Interface
*/
protected:
	virtual void Interact_Implementation(UHSInteractComponent* InteractingComponent) override;
	virtual void Interact_Finish_Implementation(UHSInteractComponent* InteractingComponent) override;
	virtual void ShowInteractiveUI_Implementation(UHSInteractComponent* InteractingComponent) override;
	virtual void HideInteractiveUI_Implementation(UHSInteractComponent* InteractingComponent) override;

	virtual bool IsInteractable_Implementation() override { return bIsInteractable; };

protected:
	UPROPERTY(EditAnywhere, Category = "Debug")
	TSubclassOf<UUserWidget> InteractWidgetClass_ForDebug;

	UPROPERTY()
	UUserWidget* InteractWidget_ForDebug;

	UPROPERTY(Replicated, VisibleAnywhere)
	bool bIsInteractable = true;
};
