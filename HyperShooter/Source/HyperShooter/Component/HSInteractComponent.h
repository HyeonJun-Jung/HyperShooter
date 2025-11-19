// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HSInteractComponent.generated.h"

class IInteractInterface;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HYPERSHOOTER_API UHSInteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHSInteractComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UFUNCTION(Server, Reliable)
	void Interact_Server();

	UFUNCTION(Server, Reliable)
	void Interact_Finish_Server();

protected:
	void FindInteractiveActor();
		
protected:
	UPROPERTY(EditAnywhere)
	float Interact_Range = 500;

	UPROPERTY(EditAnywhere)
	float Interact_Radius = 50;

	IInteractInterface* InteractiveObject;
};
