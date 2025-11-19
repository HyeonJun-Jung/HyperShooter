// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HSPlayerController.generated.h"

class UHSInteractComponent;
class UInputAction;

UCLASS()
class HYPERSHOOTER_API AHSPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UHSInteractComponent* InteractComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

protected:
	AHSPlayerController();

protected:
	virtual void SetupInputComponent() override;

protected:
	void Interact_Pressed();
	void Interact_Released();
	
protected:
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void TryServerTravel(FName InMapName);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void RegisterPlayer_Server(FName InPlayerName);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void UnregisterPlayer_Server();
};