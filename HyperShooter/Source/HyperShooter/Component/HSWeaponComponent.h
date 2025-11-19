// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Data/HSWeaponData.h"
#include "HSWeaponComponent.generated.h"

class UHSWeaponData;
class UNiagaraSystem;
class UNiagaraComponent;
class FLifetimeProperty;
class UWidgetComponent;

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

	UFUNCTION(NetMulticast, Reliable)
	void UpdateWeaponInfo_Multicast(UHSWeaponData* InWeaponDataAsset);

	UFUNCTION(Server, Reliable)
	void Fire_Server();

	UFUNCTION(NetMulticast, Reliable)
	void Fire_Multicast(FVector CameraLocation, FVector ImpactPoint);

	void PlayReloadMontage();
	void ReloadSuccess();

	void AttachWeaponToCharacter(ACharacter* character);
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
	void OnRep_Ammo();

	UFUNCTION(Client, Unreliable)
	void TargetHitSuccessed_Client();

	UFUNCTION(NetMulticast, Unreliable)
	void SpawnBulletHole_Multicast(FVector ImpactPoint, FVector ImpactNormal);

public:
	FOnAmmoUpdated Delegate_OnAmmoUpdated;
	FOnTargetHit Delegate_OnTargetHit;

public:
	UAnimMontage* GetCharacterFireMontage() const;
	UAnimMontage* GetCharacterReloadMontage() const;
	TSubclassOf<UUserWidget> GetHUDClass() const { return WeaponHUDClass; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UHSWeaponData* WeaponDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* Mesh;

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

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI)
	TSubclassOf<UUserWidget> WeaponHUDClass;
		
};
