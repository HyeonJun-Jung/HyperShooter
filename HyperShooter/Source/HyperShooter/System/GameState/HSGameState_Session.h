// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "HSStructs.h"
#include "HSGameState_Session.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUserInfoUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKillScoreUpdated);

UCLASS()
class HYPERSHOOTER_API AHSGameState_Session : public AGameStateBase
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

public:
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void OpenLevel_Multicast(FName LevelName);

public:
	void SetSessionName(FName InSessionName);
	void RegisterPlayer(APlayerController* PC, FName InPlayerName);
	void UnregisterPlayer(APlayerController* controller);
	virtual void SeamlessTravelTransitionCheckpoint(bool bToTransitionMap) override;
	virtual void PostInitializeComponents() override;
	virtual void Destroyed() override;

public:
	void UpdateKillScore(ETeamColor teamColor);
	bool IsGameFinished() const { return KillScore_Blue >= MaxKillScore || KillScore_Red >= MaxKillScore; }

protected:
	UFUNCTION()
	void OnRep_UserInfos();

	UFUNCTION()
	void OnRep_SessionName();

	UFUNCTION()
	void OnRep_KillScore_Red();

	UFUNCTION()
	void OnRep_KillScore_Blue();

private:
	UPROPERTY()
	int MaxRedTeamNum = 4;

	UPROPERTY(Replicated)
	int CurrentRedTeamNum = 0;

	UPROPERTY()
	int MaxBlueTeamNum = 4;

	UPROPERTY(Replicated)
	int CurrentBlueTeamNum = 0;

protected:
	UPROPERTY(EditAnywhere)
	int MaxKillScore = 10;

	UPROPERTY(ReplicatedUsing = OnRep_SessionName, BlueprintReadOnly)
	FName SessionName;

	UPROPERTY(ReplicatedUsing = OnRep_KillScore_Red, BlueprintReadOnly)
	int KillScore_Red;

	UPROPERTY(ReplicatedUsing = OnRep_KillScore_Blue, BlueprintReadOnly)
	int KillScore_Blue;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_UserInfos, BlueprintReadOnly)
	TArray<FUserInfo> UserInfos;

	UPROPERTY(BlueprintAssignable)
	FOnUserInfoUpdated OnUserInfoUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnKillScoreUpdated OnKillScoreUpdated;
};
