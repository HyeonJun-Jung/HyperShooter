// Copyright Epic Games, Inc. All Rights Reserved.

#include "HyperShooterGameMode.h"
#include "HyperShooterCharacter.h"
#include "UObject/ConstructorHelpers.h"

AHyperShooterGameMode::AHyperShooterGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
