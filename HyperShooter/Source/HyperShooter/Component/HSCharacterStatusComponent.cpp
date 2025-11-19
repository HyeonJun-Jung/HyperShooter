// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/HSCharacterStatusComponent.h"
#include "HSCharacterStatusComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHSCharacterStatusComponent::UHSCharacterStatusComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHSCharacterStatusComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		CurrentHP = MaxHP;
		OnRep_CurrentHP();
	}
}


// Called every frame
void UHSCharacterStatusComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHSCharacterStatusComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHSCharacterStatusComponent, CurrentHP);
}

float UHSCharacterStatusComponent::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	CurrentHP = FMath::Clamp(CurrentHP - Damage, 0, MaxHP);
	OnRep_CurrentHP();

	return CurrentHP;
}

void UHSCharacterStatusComponent::ResetStatus_Server_Implementation()
{
	CurrentHP = MaxHP;
	OnRep_CurrentHP();
}

void UHSCharacterStatusComponent::OnRep_CurrentHP()
{
	OnHPUpdated.Broadcast(MaxHP, CurrentHP);
}

