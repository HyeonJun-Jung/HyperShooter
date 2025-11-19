// Fill out your copyright notice in the Description page of Project Settings.


#include "System/PlayerState/HSPlayerState_Session.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void AHSPlayerState_Session::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHSPlayerState_Session, PlayerInfo);
}

void AHSPlayerState_Session::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (AHSPlayerState_Session* PS = Cast<AHSPlayerState_Session>(PlayerState))
	{
		PS->PlayerInfo = PlayerInfo;
	}
}

void AHSPlayerState_Session::UpdatePlayerInfo(FUserInfo& InInfo)
{
	PlayerInfo = InInfo;
}
