// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actor/InteractiveActorBase.h"
#include "Data/HSWeaponData.h"
#include "HSWeaponBase.generated.h"

class UHSWeaponData;
class UNiagaraSystem;
class UNiagaraComponent;
class FLifetimeProperty;
class UWidgetComponent;

DECLARE_MULTICAST_DELEGATE(FOnTargetHit);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAmmoUpdated, int, int);

UCLASS()
class HYPERSHOOTER_API AHSWeaponBase : public AInteractiveActorBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHSWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void InitializeWeaponData(UHSWeaponData* InWeaponData);

protected:
	UFUNCTION()
	void OnRep_WeaponDataAsset();

/*
	Interactive Interface
*/
protected:
	virtual void Interact_Implementation(UHSInteractComponent* InteractingComponent) override;
	virtual void ShowInteractiveUI_Implementation(UHSInteractComponent* InteractingComponent) override;
	virtual void HideInteractiveUI_Implementation(UHSInteractComponent* InteractingComponent) override;

public:
	EWeaponType GetWeaponType() const;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_WeaponDataAsset, EditAnywhere, BlueprintReadOnly)
	UHSWeaponData* WeaponDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UWidgetComponent* WidgetComponent;
};
