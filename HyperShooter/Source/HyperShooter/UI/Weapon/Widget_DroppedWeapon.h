// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widget_DroppedWeapon.generated.h"

class UHSWeaponData;

UCLASS()
class HYPERSHOOTER_API UWidget_DroppedWeapon : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateWeaponInfo(UHSWeaponData* InWeaponData);
	
};
