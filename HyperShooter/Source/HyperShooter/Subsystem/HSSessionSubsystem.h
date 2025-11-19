// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Delegates/Delegate.h"
#include "HSSessionSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FSessionInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FName SessionName;

	UPROPERTY(BlueprintReadOnly)
	FString OwningUserName;

	UPROPERTY(BlueprintReadOnly)
	int NumPublicConnections;

	UPROPERTY(BlueprintReadOnly)
	int NumOpenPublicConnections;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHSOnUpdateSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHSOnStartSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHSOnEndSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHSOnRegisterPlayerComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHSOnCreateSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHSOnDestroySessionComplete, bool, Successful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHSOnFindSessionsComplete, const TArray<FSessionInfo>&, SessionInfos, bool, Successful);
DECLARE_MULTICAST_DELEGATE_OneParam(FHSOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type);


UCLASS()
class HYPERSHOOTER_API UHSSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UHSSessionSubsystem();

	/*
		NetWork & Session
	*/
	UFUNCTION(BlueprintCallable, Category = "Session")
	void RegisterPlayer(FName SessionName);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateSession(FName SessionName, int PublicConnection = 4, bool bUseLan = false);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void StartSession(FName SessionName);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void UpdateSession(FName SessionName);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindSessionInfos();

	UFUNCTION(BlueprintCallable, Category = "Session")
	void DestroySession();

	// ! ! ! Should Register Player Before Join Session ! ! ! 
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinGameSession(FName InSessionName);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void TryTravelToCurrentSession(FName SessionName);

	void OnRegisterPlayerComplete(FName SessionName, const TArray<FUniqueNetIdRef>& players, bool bWasSuccessful);
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionCompleted(FName SessionName, bool Successful);
	void OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

public:
	void SessionPlayerNameUpdated();

protected:
	UPROPERTY(BlueprintAssignable)
	FHSOnRegisterPlayerComplete OnRegisterPlayerCompleteEvent;
	UPROPERTY(BlueprintAssignable)
	FHSOnCreateSessionComplete OnCreateSessionCompleteEvent;
	UPROPERTY(BlueprintAssignable)
	FHSOnUpdateSessionComplete OnUpdateSessionCompleteEvent;
	UPROPERTY(BlueprintAssignable)
	FHSOnStartSessionComplete OnStartSessionCompleteEvent;
	UPROPERTY(BlueprintAssignable)
	FHSOnEndSessionComplete OnEndSessionCompleteEvent;
	UPROPERTY(BlueprintAssignable)
	FHSOnDestroySessionComplete OnDestroySessionCompleteEvent;
	UPROPERTY(BlueprintAssignable)
	FHSOnFindSessionsComplete OnFindSessionsCompleteEvent;

	FHSOnJoinSessionComplete OnJoinGameSessionCompleteEvent;

private:
	TArray<FSessionInfo> SessionInfos;

	FOnRegisterPlayersCompleteDelegate Delegate_RegisterPlayerComplete;
	FDelegateHandle Handle_RegisterPlayerComplete;

	FOnCreateSessionCompleteDelegate Delegate_CreateSessionComplete;
	FDelegateHandle Handle_CreateSessionComplete;

	FOnStartSessionCompleteDelegate Delegate_StartSessionComplete;
	FDelegateHandle Handle_StartSessionComplete;

	FOnUpdateSessionCompleteDelegate Delegate_UpdateSessionComplete;
	FDelegateHandle Handle_UpdatetSessionComplete;

	FOnFindSessionsCompleteDelegate Delegate_FindSessionComplete;
	FDelegateHandle Handle_FindSessionComplete;

	FOnJoinSessionCompleteDelegate Delegate_JoinSessionComplete;
	FDelegateHandle Handle_JoinSessionComplete;

	FOnDestroySessionCompleteDelegate Delegate_DestroySessionComplete;
	FDelegateHandle Handle_DestroySessionComplete;

	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	FOnlineSessionSearchResult SearchResult;

public:
	FName GetCachedSessionName() const { return CachedSessionName; }

protected:
	UPROPERTY()
	FName CachedSessionName;
};
