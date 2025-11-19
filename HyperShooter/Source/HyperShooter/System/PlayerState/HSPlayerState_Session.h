// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HSStructs.h"
#include "HSPlayerState_Session.generated.h"

UCLASS()
class HYPERSHOOTER_API AHSPlayerState_Session : public APlayerState
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;

public:
	void UpdatePlayerInfo(FUserInfo& InInfo);
	const FUserInfo& GetPlayerInfo() const { return PlayerInfo; }

protected:
	UPROPERTY(Replicated, BlueprintReadOnly)
	FUserInfo PlayerInfo;
};
