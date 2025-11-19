// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/HSInteractComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Interface/InteractInterface.h"
#include "GameFramework/Character.h"
#include "HSInteractComponent.h"

// Sets default values for this component's properties
UHSInteractComponent::UHSInteractComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHSInteractComponent::BeginPlay()
{
	Super::BeginPlay();

	AController* controller = Cast<AController>(GetOwner());
	if (controller && controller->IsLocalPlayerController())
	{
		FTimerHandle handle;
		GetWorld()->GetTimerManager().SetTimer(handle, this, &UHSInteractComponent::FindInteractiveActor, 0.05f, true);
	}
}

void UHSInteractComponent::Interact_Server_Implementation()
{
	AController* controller = Cast<AController>(GetOwner());
	if (!controller) return;

	APawn* pawn = controller->GetPawn();
	if (!pawn) return;

	UCameraComponent* camera = pawn->GetComponentByClass<UCameraComponent>();
	if (!camera) return;


	// Normal Interact
	// Find Interactable Actor & Try Interact
	else
	{
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(pawn);

		FVector start = camera->GetComponentLocation();
		FRotator ControllerRot = controller->GetControlRotation();
		FVector ControllerForwardVec = UKismetMathLibrary::GetForwardVector(ControllerRot);
		FVector end = start + Interact_Range * ControllerForwardVec;

		ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel1);

		FHitResult hitResult;
		UKismetSystemLibrary::SphereTraceSingle(pawn, start, end, Interact_Radius,
			TraceType, false, IgnoreActors,
			EDrawDebugTrace::None, hitResult, true);

		if (hitResult.bBlockingHit)
		{
			InteractiveObject = Cast<IInteractInterface>(hitResult.GetActor());
			{
				if (InteractiveObject && IInteractInterface::Execute_IsInteractable(hitResult.GetActor()))
				{
					IInteractInterface::Execute_Interact(hitResult.GetActor(), this);
				}
			}
		}
	}
}

void UHSInteractComponent::Interact_Finish_Server_Implementation()
{
	if (InteractiveObject)
	{
		UObject* object = Cast<UObject>(InteractiveObject);
		if (object)
			IInteractInterface::Execute_Interact_Finish(object, this);
	}
}

void UHSInteractComponent::FindInteractiveActor()
{
	AController* controller = Cast<AController>(GetOwner());
	if (!controller) return;

	APawn* pawn = controller->GetPawn();
	if (!pawn) return;

	UCameraComponent* camera = pawn->GetComponentByClass<UCameraComponent>();
	if (!camera) return;

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(pawn);

	FVector start = camera->GetComponentLocation(); // pawn->GetActorLocation();
	FRotator ControllerRot = controller->GetControlRotation();
	FVector ControllerForwardVec = UKismetMathLibrary::GetForwardVector(ControllerRot);
	FVector end = start + Interact_Range * ControllerForwardVec;
	// FVector end = start + Interact_Range * pawn->GetActorForwardVector();

	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel1);

	FHitResult hitResult;
	UKismetSystemLibrary::SphereTraceSingle(pawn, start, end, Interact_Radius,
		TraceType, false, IgnoreActors,
		EDrawDebugTrace::None, hitResult, true);

	if (hitResult.bBlockingHit)
	{
		IInteractInterface* newInteractiveObject = Cast<IInteractInterface>(hitResult.GetActor());

		// Show Interactable UI 
		if (newInteractiveObject && InteractiveObject != newInteractiveObject)
		{
			if (InteractiveObject)
				IInteractInterface::Execute_HideInteractiveUI(Cast<UObject>(InteractiveObject), this);

			InteractiveObject = newInteractiveObject;

			if (InteractiveObject)
				IInteractInterface::Execute_ShowInteractiveUI(Cast<UObject>(InteractiveObject), this);
		}
	}
	else
	{
		if (InteractiveObject)
		{
			UObject* object = Cast<UObject>(InteractiveObject);
			if (object)
				IInteractInterface::Execute_HideInteractiveUI(object, this);
		}

		//if (bIsHolding)
		//{
		//	/*
		//		Should Cancel Interact In Server !  ! !
		//	*/
		//	Interact_Finish_Server();
		//	// InteractiveObject->Interact_Finish(this);
		//	FinishInteractHold_Client();
		//}

		InteractiveObject = nullptr;
	}
}

