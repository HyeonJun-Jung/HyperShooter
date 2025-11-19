// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/HSPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "System/GameState/HSGameState_Session.h"
#include "Component/HSInteractComponent.h"
#include "Player/HSLocalPlayer.h"
#include "Subsystem/HSSessionSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"

AHSPlayerController::AHSPlayerController()
{
	InteractComponent = CreateDefaultSubobject<UHSInteractComponent>(FName("InteractComponent"));
}

void AHSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AHSPlayerController::Interact_Pressed);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AHSPlayerController::Interact_Released);
	}
}

void AHSPlayerController::Interact_Pressed()
{
	if (InteractComponent)
	{
		InteractComponent->Interact_Server();
	}
}


void AHSPlayerController::Interact_Released()
{
	if (InteractComponent)
	{
		InteractComponent->Interact_Finish_Server();
	}
}

void AHSPlayerController::RegisterPlayer_Server_Implementation(FName InPlayerName)
{
	AHSGameState_Session* gameState = Cast<AHSGameState_Session>(UGameplayStatics::GetGameState(this));
	if (gameState)
	{
		gameState->RegisterPlayer(this, InPlayerName);
	}
}

void AHSPlayerController::UnregisterPlayer_Server_Implementation()
{
	AHSGameState_Session* gameState = Cast<AHSGameState_Session>(UGameplayStatics::GetGameState(this));
	if (gameState)
	{
		gameState->UnregisterPlayer(this);
	}

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub) return;

	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface.IsValid()) return;

	UHSSessionSubsystem* SessionSubSystem = GetGameInstance()->GetSubsystem<UHSSessionSubsystem>();

	if (UHSLocalPlayer* localPlayer = Cast<UHSLocalPlayer>(GetLocalPlayer()))
	{
		if (!SessionInterface->UnregisterPlayer(SessionSubSystem->GetCachedSessionName(), *localPlayer->GetPreferredUniqueNetId()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Client : ASTPlayerController Failed To Unregister Player To Session."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Client : ASTPlayerController Success To Unregister Player To Session."));
		}
	}
	else
	{
		if (!SessionInterface->UnregisterPlayer(SessionSubSystem->GetCachedSessionName(), *PlayerState->GetUniqueId()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Client : ASTPlayerController Failed To Unregister Player To Session."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Client : ASTPlayerController Success To Unregister Player To Session."));
		}
	}
}

void AHSPlayerController::TryServerTravel_Implementation(FName InMapName)
{
	AGameModeBase* gameMode = UGameplayStatics::GetGameMode(this);
	if (gameMode)
	{
		gameMode->bUseSeamlessTravel = true;
		GetWorld()->ServerTravel(InMapName.ToString());
	}
}
