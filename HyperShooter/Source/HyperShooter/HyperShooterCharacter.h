// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Data/HSWeaponData.h"
#include "HyperShooterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AHSWeaponBase;
class UWidget_HUD;
class UHSWeaponData;
struct FInputActionValue;
class UHSCharacterStatusComponent;
class UHSCameraModeComponent;
class UHSWeaponComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterDied, ACharacter*);

UENUM(BlueprintType)
enum EHSHitDirection
{
	Front,
	Back,
	Left,
	Right,
};

UCLASS(config=Game)
class AHyperShooterCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/*
		Additional Actions
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DropWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FirearmSwapAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* GrenadeSwapAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* KnifeSwapAction;

	
	/*
		Custom Components
	*/
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UHSCharacterStatusComponent* StatusComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UHSCameraModeComponent* CameraModeComponent;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UHSWeaponComponent* WeaponComponent;

	/*
		Default Weapon
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AHSWeaponBase> DefaultWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	UHSWeaponData* DefaultWeaponData;

	UPROPERTY(EditAnywhere, Category = Dash)
	float MaxWalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = Dash)
	float MaxDashSpeed = 900.f;

	UPROPERTY()
	UWidget_HUD* HUDWidget;

	UPROPERTY()
	FTimerHandle FireHandle;

	UPROPERTY()
	bool bCharacterDied = false;
	
public:
	FOnCharacterDied OnCharacterDied;

public:
	AHyperShooterCharacter();
	

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

	virtual void Destroyed() override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

protected:
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

public:
	void WeaponStateUpdated(EWeaponState NewWeaponState);
	void WeaponInfoUpdated();

public:
	UFUNCTION(NetMulticast, UnReliable, BlueprintCallable)
	void PlayMontage_Multicast(UAnimMontage* InMontage);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void UpdateWeaponInfo_Server(UHSWeaponData* InWeaponData);

	UFUNCTION()
	void OnAmmoUpdated(int MaxAmmo, int CurrentAmmo);

protected:
/*
	Dash
*/
	void DashPressed();

	void DashReleased();

	UFUNCTION(Server, Reliable)
	void DashPressed_Server();

	UFUNCTION(Server, Reliable)
	void DashReleased_Server();

/*
	Crouch	
*/
	UFUNCTION(Server, Reliable)
	void CrouchPressed_Server();

	UFUNCTION(Server, Reliable)
	void CrouchReleased_Server();

	UFUNCTION(NetMulticast, Reliable)
	void CrouchPressed_Multicast();

	UFUNCTION(NetMulticast, Reliable)
	void CrouchReleased_Multicast();

/*
	Swap Weapon
*/
	void SwapWeapon_Firearm();
	void SwapWeapon_Grenade();
	void SwapWeapon_Knife();

/*
	Drop Weapon
*/
	UFUNCTION(Server, Reliable)
	void DropWeapon_Server();



/*
	Fire
*/
	void FirePressed();

	void PlayFireAnimationWithWeapon();

	UFUNCTION(Server, Reliable)
	void Fire_Server();

	UFUNCTION(NetMulticast, Reliable)
	void Fire_Multicast();

/*
	Fire Relaese
*/
	void FireReleased();

	UFUNCTION(Server, Reliable)
	void FireReleased_Server();

	UFUNCTION(NetMulticast, Reliable)
	void FireReleased_Multicast();

/*
	Reload
*/
	UFUNCTION(Server, Reliable)
	void Reload_Server();

	UFUNCTION(NetMulticast, Reliable)
	void Reload_Multicast();

/*
	Fire Hit Callback
*/
	UFUNCTION()
	void OnFireHit();

/*
	Hit & Died
*/
	EHSHitDirection GetHitReactDirection(const FVector& ImpactPoint, AActor* DamageCauser) const;

	UFUNCTION(NetMulticast, Reliable)
	void CharacterDamaged_Multicast(AActor* DamageCauser, EHSHitDirection hitDirection);

	UFUNCTION(BlueprintImplementableEvent)
	void PlayHitMontage(EHSHitDirection hitDirection);

	UFUNCTION(NetMulticast, Reliable)
	void CharacterDied_Multicast(EHSHitDirection hitDirection);

	UFUNCTION(BlueprintImplementableEvent)
	void PlayDeathMontage(EHSHitDirection hitDirection);

public:
	void ReloadSuccess();


public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

