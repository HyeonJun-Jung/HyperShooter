// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/HSCameraModeComponent.h"
#include "Component/Camera/CameraMode.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UHSCameraModeComponent::UHSCameraModeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHSCameraModeComponent::BeginPlay()
{
	Super::BeginPlay();

	SpringArm = GetOwner()->FindComponentByClass<USpringArmComponent>();
	InitCameraMode();
}


// Called every frame
void UHSCameraModeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentMode)
	{
		CurrentMode->Tick(DeltaTime);
	}
}

void UHSCameraModeComponent::InitCameraMode()
{
	if (DefaultCameraMode)
	{
		SetCameraMode(DefaultCameraMode);
	}
}

void UHSCameraModeComponent::SetCameraMode(TSubclassOf<UCameraMode> NewModeClass)
{
	ACharacter* character = Cast<ACharacter>(GetOwner());
	if (character)
	{
		if (UCharacterMovementComponent* characterMovement = character->GetCharacterMovement())
		{
			characterMovement->StopMovementImmediately();
		}
	}

	if (CurrentMode)
	{
		CurrentMode->OnExit();
	}

	CurrentMode = NewObject<UCameraMode>(this, NewModeClass);
	if (CurrentMode)
	{
		CurrentMode->OnEnter(this);
	}
}

