// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractInterface.generated.h"

class UHSInteractComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HYPERSHOOTER_API IInteractInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent)
	void Interact(UHSInteractComponent* InteractingComponent);
	UFUNCTION(BlueprintNativeEvent)
	void Interact_Finish(UHSInteractComponent* InteractingComponent);
	UFUNCTION(BlueprintNativeEvent)
	void ShowInteractiveUI(UHSInteractComponent* InteractingComponent);
	UFUNCTION(BlueprintNativeEvent)
	void HideInteractiveUI(UHSInteractComponent* InteractingComponent);

	UFUNCTION(BlueprintNativeEvent)
	bool IsInteractable();
};
