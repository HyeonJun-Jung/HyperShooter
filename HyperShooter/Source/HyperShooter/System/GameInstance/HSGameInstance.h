// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HSStructs.h"
#include "HSGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class HYPERSHOOTER_API UHSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SaveLocalPlayerInfo(FName InPlayerName);

	void SaveUserInfos(const TArray<FUserInfo>& InUserInfos);
	void GetSavedUserInfoS(TArray<FUserInfo>& OutUserInfos) const;

protected:
	UPROPERTY(BlueprintReadOnly)
	FName LocalPlayerName;

	UPROPERTY()
	TArray<FUserInfo> CachedUserInfos;
};
