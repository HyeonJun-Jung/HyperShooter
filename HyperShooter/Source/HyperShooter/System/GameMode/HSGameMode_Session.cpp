// Fill out your copyright notice in the Description page of Project Settings.


#include "System/GameMode/HSGameMode_Session.h"
#include "System/GameState/HSGameState_Session.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystemTypes.h"
#include "System/GameState/HSGameState_Session.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "System/PlayerState/HSPlayerState_Session.h"
#include "Kismet/KismetMathLibrary.h"
#include "HyperShooterCharacter.h"
#include "System/PlayerState/HSPlayerState_Session.h"
#include "Player/HSLocalPlayer.h"
#include "Subsystem/HSSessionSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"

void AHSGameMode_Session::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass(this, PlayerStartClass_RedTeam, actors);
	for (AActor* actor : actors)
	{
		if (APlayerStart* PS = Cast<APlayerStart>(actor))
		{
			RedTeamPlayerStarts.Add(PS);
		}
	}

	UGameplayStatics::GetAllActorsOfClass(this, PlayerStartClass_BlueTeam, actors);
	for (AActor* actor : actors)
	{
		if (APlayerStart* PS = Cast<APlayerStart>(actor))
		{
			BlueTeamPlayerStarts.Add(PS);
		}
	}
}

void AHSGameMode_Session::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// GameMode exist only on Server

	AHSGameState_Session* gameState = GetGameState<AHSGameState_Session>();
	if (gameState)
	{

	}

	/*
		Register Player To Session
	*/

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub) return;

	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface.IsValid()) return;

	UHSSessionSubsystem* SessionSubSystem = GetGameInstance()->GetSubsystem<UHSSessionSubsystem>();

	if (UHSLocalPlayer* localPlayer = Cast<UHSLocalPlayer>(NewPlayer->GetLocalPlayer()))
	{
		if (!SessionInterface->RegisterPlayer(SessionSubSystem->GetCachedSessionName(), *localPlayer->GetPreferredUniqueNetId(), false))
		{
			UE_LOG(LogTemp, Warning, TEXT("Client : ASTPlayerController Failed To Register Player To Session."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Client : ASTPlayerController Success To Register Player To Session."));
		}
	}
	else
	{
		if (!SessionInterface->RegisterPlayer(SessionSubSystem->GetCachedSessionName(), *NewPlayer->PlayerState->GetUniqueId(), false))
		{
			UE_LOG(LogTemp, Warning, TEXT("Client : ASTPlayerController Failed To Register Player To Session."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Client : ASTPlayerController Success To Register Player To Session."));
		}
	}
}

void AHSGameMode_Session::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	// Move Logic To PlayerController Because It's hard to Distinguish Player is in OpenLevel & ServerTravel!

	/*AHSGameState_Session* gameState = GetGameState<AHSGameState_Session>();
	if (gameState)
	{
		gameState->UnregisterPlayer(Cast<APlayerController>(Exiting));
	}*/
}

void AHSGameMode_Session::GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList)
{
	Super::GetSeamlessTravelActorList(bToTransition, ActorList);
}

void AHSGameMode_Session::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	BindDelegateWithPlayer(NewPlayer);
}

void AHSGameMode_Session::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	BindDelegateWithPlayer(NewPlayer);
}

AActor* AHSGameMode_Session::ChoosePlayerStart_Implementation(AController* Player)
{
	if (AHSPlayerState_Session* PlayerState = Player->GetPlayerState<AHSPlayerState_Session>())
	{
		const FUserInfo& playerInfo = PlayerState->GetPlayerInfo();

		if (playerInfo.TeamColor == ETeamColor::ERed && RedTeamPlayerStarts.Num() > 0)
		{
			int rand = UKismetMathLibrary::RandomIntegerInRange(0, RedTeamPlayerStarts.Num() - 1);
			return RedTeamPlayerStarts[rand];
		}
		else if(playerInfo.TeamColor == ETeamColor::EBlue && BlueTeamPlayerStarts.Num() > 0)
		{
			int rand = UKismetMathLibrary::RandomIntegerInRange(0, BlueTeamPlayerStarts.Num() - 1);
			return BlueTeamPlayerStarts[rand];
		}
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

void AHSGameMode_Session::BindDelegateWithPlayer(AController* Player)
{
	if (AHyperShooterCharacter* character = Cast<AHyperShooterCharacter>(Player->GetPawn()))
	{
		character->OnCharacterDied.RemoveAll(this);
		character->OnCharacterDied.AddUObject(this, &AHSGameMode_Session::CharacterDiedCallback);
	}
}

void AHSGameMode_Session::CharacterDiedCallback(ACharacter* character)
{
	if (character)
	{
		AHSPlayerState_Session* PS = Cast<AHSPlayerState_Session>(character->GetPlayerState());
		AHSGameState_Session* GS = GetGameState<AHSGameState_Session>();
		if (PS && GS)
		{
			const FUserInfo& info = PS->GetPlayerInfo();
			GS->UpdateKillScore(info.TeamColor);

			if (GS->IsGameFinished())
			{
				FTimerHandle TravelHandle;
				GetWorldTimerManager().SetTimer(TravelHandle,
					[this]()
					{
						GetWorld()->ServerTravel(NextMapName.ToString());
					}, 5.f, false);
			}
		}
	}
}
