// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Data/HSWeaponData.h"
#include "HSWeaponComponent.generated.h"

class UHSWeaponData;
class AHSWeaponBase;
class UNiagaraSystem;
class UNiagaraComponent;
class FLifetimeProperty;
class UWidgetComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnWeaponStateUpdated, EWeaponState);
DECLARE_MULTICAST_DELEGATE(FOnWeaponUpdated);
DECLARE_MULTICAST_DELEGATE(FOnTargetHit);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAmmoUpdated, int, int);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HYPERSHOOTER_API UHSWeaponComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHSWeaponComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void OnRegister() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	void InitializeWeaponData();

/*
	Weapon
*/
public:
	UFUNCTION(Server, Reliable)
	void UpdateWeaponInfo_Server(UHSWeaponData* InWeaponDataAsset);

	UFUNCTION(Server, Reliable)
	void SwapWeapon_Server(EWeaponState InWeaponState);

	UFUNCTION(Server, Reliable)
	void Fire_Server();

	UFUNCTION(Server, Reliable)
	void Fire_Firearm_Server();

	UFUNCTION(Server, Reliable)
	void Fire_Grenade_Server();

	UFUNCTION()
	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void Fire_Knife_Server();

	UFUNCTION(NetMulticast, Reliable)
	void Fire_Multicast(FVector CameraLocation, FVector ImpactPoint);

	void PlayReloadMontage();
	void ReloadSuccess();

	void DropWeaponFromOwner();

public:
	EWeaponType GetWeaponType() const;
	bool HasEnoughAmmo() const { return CurrentAmmo > 0; }
	bool CanAutoFire() const { return AutoFire; }
	float GetFireLatency() const { return FireLatency; }

	/*
		FX
	*/
protected:
	void RecoilTick(float DeltaTime);
	void ApplyRecoil();
	void ShowShellEject();
	void ShowMuzzleFlash(FVector cameraLocation, FVector ImpactPoint);
	void ShowTracer(FVector cameraLocation, FVector ImpactPoint);

	/*
	*
	*
		Call back
	*/
protected:
	UFUNCTION()
	void OnRep_WeaponState();

	UFUNCTION() 
	void OnRep_WeaponDataAsset();

	UFUNCTION()
	void OnRep_Ammo();

	UFUNCTION(Client, Unreliable)
	void TargetHitSuccessed_Client();

	UFUNCTION(NetMulticast, Unreliable)
	void SpawnBulletHole_Multicast(FVector ImpactPoint, FVector ImpactNormal);

public:
	FOnWeaponStateUpdated Delegate_OnWeaponStateUpdated;
	FOnWeaponUpdated Delegate_OnWeaponUpdated;
	FOnAmmoUpdated Delegate_OnAmmoUpdated;
	FOnTargetHit Delegate_OnTargetHit;

public:
	EWeaponState GetWeaponState() const { return CurrentWeaponState; }
	const UHSWeaponData* GetWeaponData() const { return WeaponDataAsset; }
	UAnimMontage* GetCharacterEquipMontage() const;
	UAnimMontage* GetCharacterFireMontage() const;
	UAnimMontage* GetCharacterReloadMontage() const;
	TSubclassOf<UUserWidget> GetHUDClass() const;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EWeaponState> CurrentWeaponState;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponDataAsset, EditAnywhere, BlueprintReadOnly)
	UHSWeaponData* WeaponDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AHSWeaponBase> DropWeaponClass;

	UPROPERTY(ReplicatedUsing = OnRep_Ammo, EditAnywhere, BlueprintReadOnly, Category = Ammo)
	int MaxAmmo = 10;

	UPROPERTY(ReplicatedUsing = OnRep_Ammo, VisibleAnywhere, BlueprintReadOnly, Category = Ammo)
	int CurrentAmmo = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Performance)
	float MaxRange = 2000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Performance)
	bool AutoFire = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Performance)
	float DefaultDamage = 10.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Performance)
	float FireLatency = 0.25f;

	UPROPERTY()
	int RecoilIndex = 0;

	UPROPERTY()
	float TimeSinceFire = 0;

	UPROPERTY()
	FVector2D CurrentRecoilOffset;

	UPROPERTY()
	FVector2D TargetRecoilOffset;

/*
	FX
*/
protected:
	UPROPERTY()
	UNiagaraComponent* NC_ShellEject;

	UPROPERTY()
	bool ShellEjectTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = FX)
	UNiagaraSystem* ShellEjectSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = FX)
	UStaticMesh* ShellEjectMesh;

	UPROPERTY()
	UNiagaraComponent* NC_MuzzleFlash;

	UPROPERTY()
	bool MuzzleFlashTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = FX)
	UNiagaraSystem* MuzzleFlashSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = FX)
	FVector MuzzlePosition;

	UPROPERTY()
	UNiagaraComponent* NC_Tracer;

	UPROPERTY()
	bool TracerTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = FX)
	UNiagaraSystem* TracerSystem;

/*	
	Grenade
*/
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Grenade)
	TSubclassOf<AActor> GrenadeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Grenade)
	UAnimMontage* Montage_GrenadeThrowLoop;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Grenade)
	UAnimMontage* Montage_GrenadeThrow;

	UPROPERTY()
	AActor* SpawnedGrenade;
		
};
