// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HSStructs.generated.h"

class APlayerController;

UENUM(BlueprintType)
enum ETeamColor
{
	EBlue,
	ERed,
};

USTRUCT(BlueprintType)
struct FUserInfo
{
	GENERATED_BODY();

	bool operator==(const FUserInfo& other) const
	{
		return PlayerController == other.PlayerController || PlayerNetId == other.PlayerNetId;
	}

	UPROPERTY(BlueprintReadOnly)
	APlayerController* PlayerController;

	UPROPERTY(BlueprintReadOnly)
	FUniqueNetIdRepl PlayerNetId;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<ETeamColor> TeamColor;

	UPROPERTY(BlueprintReadOnly)
	FName PlayerName;
};