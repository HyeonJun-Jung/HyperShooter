// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/InteractiveActorBase.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AInteractiveActorBase::AInteractiveActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AInteractiveActorBase::BeginPlay()
{
	Super::BeginPlay();
}

void AInteractiveActorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInteractiveActorBase, bIsInteractable);
}

void AInteractiveActorBase::Interact_Implementation(UHSInteractComponent* InteractingComponent)
{
	HideInteractiveUI_Implementation(InteractingComponent);
}

void AInteractiveActorBase::Interact_Finish_Implementation(UHSInteractComponent* InteractingComponent)
{
}

void AInteractiveActorBase::ShowInteractiveUI_Implementation(UHSInteractComponent* InteractingComponent)
{
	UE_LOG(LogTemp, Display, TEXT("ShowInteractiveUI_Implementation"));
	if (!bIsInteractable)
		return;

	if (InteractWidget_ForDebug)
	{
		InteractWidget_ForDebug->RemoveFromParent();
	}

	if (InteractWidgetClass_ForDebug)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (PC)
		{
			InteractWidget_ForDebug = CreateWidget<UUserWidget>(PC, InteractWidgetClass_ForDebug);
			if (InteractWidget_ForDebug)
			{
				InteractWidget_ForDebug->AddToViewport();
			}
		}
	}
}

void AInteractiveActorBase::HideInteractiveUI_Implementation(UHSInteractComponent* InteractingComponent)
{
	UE_LOG(LogTemp, Display, TEXT("HideInteractiveUI_Implementation"));
	if (InteractWidget_ForDebug)
	{
		InteractWidget_ForDebug->RemoveFromParent();
	}
}


