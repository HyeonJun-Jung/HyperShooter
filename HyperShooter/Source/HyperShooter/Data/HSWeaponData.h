// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetUserData.h"
#include "Engine/DataAsset.h"
#include "HSWeaponData.generated.h"

class UNiagaraSystem;

UENUM(BlueprintType)
enum EWeaponType
{
	Pistol,
	Rifle,
	Shotgun
};

UCLASS()
class HYPERSHOOTER_API UHSWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/*
		Ammo & Performance
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon)
	TEnumAsByte<EWeaponType> WeaponType = EWeaponType::Pistol;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ammo)
	int MaxAmmo = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ammo)
	int CurrentAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Performance)
	bool AutoFire = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Performance)
	float DefaultDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Performance)
	float FireLatency = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Performance)
	float MaxRange = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Performance)
	UCurveFloat* RecoilPitchCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Performance)
	UCurveFloat* RecoilYawCurve;
	
	/*
		Mesh
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh)
	USkeletalMesh* Mesh;

	/*
		AnimInstance
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AnimClass)
	TSubclassOf<UAnimInstance> CharacterAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AnimClass)
	TSubclassOf<UAnimInstance> WeaponAnimClass;

	/*
		Montages 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage)
	UAnimMontage* CharacterEquipMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage)
	UAnimMontage* CharacterFireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage)
	UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage)
	UAnimMontage* CharacterReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage)
	UAnimMontage* ReloadMontage;

	/*
		Bullet Hole Decal
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = FX)
	UMaterial* BulletHoleDecalMaterial;

	/*
		Shell Eject FX
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = FX)
	UNiagaraSystem* ShellEjectSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = FX)
	UStaticMesh* ShellEjectMesh;

	/*
		Muzzle Flash FX
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = FX)
	UNiagaraSystem* MuzzleFlashSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = FX)
	FVector MuzzlePosition;

	/*
		Tracer FX
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = FX)
	UNiagaraSystem* TracerSystem;
};
