// Fill out your copyright notice in the Description page of Project Settings.


#include "System/GameState/HSGameState_Session.h"
#include "Net/UnrealNetwork.h"
#include "Subsystem/HSSessionSubsystem.h"
#include "System/PlayerState/HSPlayerState_Session.h"
#include "System/GameInstance/HSGameInstance.h"
#include "Kismet/GameplayStatics.h"

void AHSGameState_Session::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHSGameState_Session, UserInfos);
	DOREPLIFETIME(AHSGameState_Session, SessionName);
	DOREPLIFETIME(AHSGameState_Session, KillScore_Red);
	DOREPLIFETIME(AHSGameState_Session, KillScore_Blue);
}

void AHSGameState_Session::OpenLevel_Multicast_Implementation(FName LevelName)
{
	UGameplayStatics::OpenLevel(this, LevelName);
}

void AHSGameState_Session::SetSessionName(FName InSessionName)
{
	SessionName = InSessionName;
}

void AHSGameState_Session::RegisterPlayer(APlayerController* PC, FName InPlayerName)
{
	for (auto& info : UserInfos)
	{
		if (info.PlayerNetId == PC->GetPlayerState<APlayerState>()->GetUniqueId())
		{
			return;
		}
	}

	FUserInfo NewUserInfo;

	if (CurrentRedTeamNum < CurrentBlueTeamNum)
	{
		CurrentRedTeamNum++;
		NewUserInfo.TeamColor = ETeamColor::ERed;
	}
	else
	{
		CurrentBlueTeamNum++;
		NewUserInfo.TeamColor = ETeamColor::EBlue;
	}

	NewUserInfo.PlayerNetId = PC->GetPlayerState<APlayerState>()->GetUniqueId();
	NewUserInfo.PlayerController = PC;
	NewUserInfo.PlayerName = InPlayerName;

	UserInfos.Add(NewUserInfo);

	AHSPlayerState_Session* PlayerState = PC->GetPlayerState<AHSPlayerState_Session>();
	if (PlayerState)
	{
		PlayerState->UpdatePlayerInfo(NewUserInfo);
	}

	OnRep_UserInfos();
}

void AHSGameState_Session::UnregisterPlayer(APlayerController* PC)
{
	if (!PC) return;

	for (int idx = 0; idx < UserInfos.Num(); idx++)
	{
		auto& info = UserInfos[idx];
		APlayerState* PS = PC->GetPlayerState<APlayerState>();
		if (PS && info.PlayerNetId == PS->GetUniqueId())
		{
			UserInfos.RemoveAt(idx, 1, true);

			OnRep_UserInfos();
			return;
		}
	}
}

void AHSGameState_Session::SeamlessTravelTransitionCheckpoint(bool bToTransitionMap)
{
	Super::SeamlessTravelTransitionCheckpoint(bToTransitionMap);

	if (HasAuthority() && UserInfos.Num() > 0)
	{
		if (UHSGameInstance* gameInst = Cast<UHSGameInstance>(GetGameInstance()))
		{
			gameInst->SaveUserInfos(UserInfos);
		}
	}
}

void AHSGameState_Session::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		if (UHSGameInstance* gameInst = Cast<UHSGameInstance>(GetGameInstance()))
		{
			gameInst->GetSavedUserInfoS(UserInfos);
			OnRep_UserInfos();

			UHSSessionSubsystem* SessionSystem = gameInst->GetSubsystem<UHSSessionSubsystem>();
			if (SessionSystem)
			{
				SetSessionName(SessionSystem->GetCachedSessionName());
			}
		}
	}
}

void AHSGameState_Session::Destroyed()
{
	if (HasAuthority())
	{
		if (UHSGameInstance* gameInst = Cast<UHSGameInstance>(GetGameInstance()))
		{
			gameInst->SaveUserInfos(UserInfos);
		}
	}

	Super::Destroyed();
}

void AHSGameState_Session::UpdateKillScore(ETeamColor teamColor)
{
	if (teamColor == ETeamColor::ERed)
	{
		KillScore_Red++;
		OnRep_KillScore_Red();
	}
	else
	{
		KillScore_Blue++;
		OnRep_KillScore_Blue();
	}
}

void AHSGameState_Session::OnRep_UserInfos()
{
	OnUserInfoUpdated.Broadcast();
}

void AHSGameState_Session::OnRep_SessionName()
{
}

void AHSGameState_Session::OnRep_KillScore_Red()
{
	UE_LOG(LogTemp, Display, TEXT("OnRep_KillScore_Red"));
	OnKillScoreUpdated.Broadcast();
}

void AHSGameState_Session::OnRep_KillScore_Blue()
{
	UE_LOG(LogTemp, Display, TEXT("OnRep_KillScore_Blue"));
	OnKillScoreUpdated.Broadcast();
}
