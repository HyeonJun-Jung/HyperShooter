// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HSStructs.h"
#include "HSGameMode_Session.generated.h"

class APlayerStart;

UCLASS()
class HYPERSHOOTER_API AHSGameMode_Session : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage);
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

protected:
	void BindDelegateWithPlayer(AController* Player);

	UFUNCTION()
	void CharacterDiedCallback(ACharacter* character);

protected:
	UPROPERTY(EditAnywhere)
	FName NextMapName;

	UPROPERTY(EditAnywhere)
	TSubclassOf<APlayerStart> PlayerStartClass_RedTeam;

	UPROPERTY(EditAnywhere)
	TSubclassOf<APlayerStart> PlayerStartClass_BlueTeam;

	UPROPERTY()
	TArray<APlayerStart*> RedTeamPlayerStarts;

	UPROPERTY()
	TArray<APlayerStart*> BlueTeamPlayerStarts;
};
