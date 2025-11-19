// Fill out your copyright notice in the Description page of Project Settings.


#include "System/GameInstance/HSGameInstance.h"

void UHSGameInstance::SaveLocalPlayerInfo(FName InPlayerName)
{
	LocalPlayerName = InPlayerName;
}

void UHSGameInstance::SaveUserInfos(const TArray<FUserInfo>& InUserInfos)
{
	CachedUserInfos = InUserInfos;
}

void UHSGameInstance::GetSavedUserInfoS(TArray<FUserInfo>& OutUserInfos) const
{
	OutUserInfos = CachedUserInfos;
}
