// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widget_HUD.generated.h"

/**
 * 
 */
UCLASS()
class HYPERSHOOTER_API UWidget_HUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void PlayFireAnimation();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayImpactAnimation();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowHitIndicator(FVector HitLocation);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateAmmo(int MaxAmmo, int CurrentAmmo);
};
