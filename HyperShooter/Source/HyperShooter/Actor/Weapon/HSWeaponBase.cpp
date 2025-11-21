// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Weapon/HSWeaponBase.h"
#include "HSWeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Data/HSWeaponData.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/DecalComponent.h"
#include "Components/WidgetComponent.h"
#include "Component/HSInteractComponent.h"
#include "HyperShooterCharacter.h"
#include "UI/Weapon/Widget_DroppedWeapon.h"

// Sets default values
AHSWeaponBase::AHSWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("Mesh"));
	SetRootComponent(Mesh);

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(FName("WidgetComponent"));
	WidgetComponent->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void AHSWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	WidgetComponent->SetVisibility(false);
}

void AHSWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHSWeaponBase, WeaponDataAsset);
}

void AHSWeaponBase::InitializeWeaponData(UHSWeaponData* InWeaponData)
{
	WeaponDataAsset = InWeaponData;

	OnRep_WeaponDataAsset();
}

void AHSWeaponBase::OnRep_WeaponDataAsset()
{
	if (WeaponDataAsset)
	{
		Mesh->SetSkeletalMesh(WeaponDataAsset->Mesh);
	}
}

void AHSWeaponBase::Interact_Implementation(UHSInteractComponent* InteractingComponent)
{
	Super::Interact_Implementation(InteractingComponent);

	AController* PC = Cast<AController>(InteractingComponent->GetOwner());
	if (PC)
	{
		AHyperShooterCharacter* character = Cast<AHyperShooterCharacter>(PC->GetPawn());
		if (character)
		{
			character->UpdateWeaponInfo_Server(WeaponDataAsset);
		}
	}

	Destroy();
}

void AHSWeaponBase::ShowInteractiveUI_Implementation(UHSInteractComponent* InteractingComponent)
{
	if (!bIsInteractable) return;

	if (WidgetComponent)
	{
		WidgetComponent->SetVisibility(true);

		if (UWidget_DroppedWeapon* widget = Cast<UWidget_DroppedWeapon>(WidgetComponent->GetWidget()))
		{
			widget->UpdateWeaponInfo(WeaponDataAsset);
		}
	}
}

void AHSWeaponBase::HideInteractiveUI_Implementation(UHSInteractComponent* InteractingComponent)
{
	if (WidgetComponent)
	{
		WidgetComponent->SetVisibility(false);
	}
}

EWeaponType AHSWeaponBase::GetWeaponType() const
{
	if (WeaponDataAsset)
	{
		return WeaponDataAsset->WeaponType;
	}

	return EWeaponType::Pistol;
}
