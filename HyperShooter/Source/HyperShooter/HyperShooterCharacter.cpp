// Copyright Epic Games, Inc. All Rights Reserved.

#include "HyperShooterCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Data/HSWeaponData.h"
#include "Actor/Weapon/HSWeaponBase.h"
#include "UI/Widget_HUD.h"
#include "Character/HSAnimInstanceBase.h"
#include "Net/UnrealNetwork.h"
#include "Component/HSCharacterStatusComponent.h"
#include "Component/HSCameraModeComponent.h"
#include "Component/HSWeaponComponent.h"
#include "Engine/DamageEvents.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AHyperShooterCharacter

AHyperShooterCharacter::AHyperShooterCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	WeaponComponent = nullptr;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	StatusComponent = CreateDefaultSubobject<UHSCharacterStatusComponent>(FName("StatusComponent"));

	CameraModeComponent = CreateDefaultSubobject<UHSCameraModeComponent>(FName("CameraModeComponent"));

	WeaponComponent = CreateDefaultSubobject<UHSWeaponComponent>(FName("WeaponComponent"));
	WeaponComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName("weapon_r"));
}

void AHyperShooterCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	WeaponComponent->Delegate_OnWeaponUpdated.AddUObject(this, &AHyperShooterCharacter::WeaponInfoUpdated);

	UpdateWeaponInfo_Server(DefaultWeaponData);
}

void AHyperShooterCharacter::Destroyed()
{
	Super::Destroy();
}

void AHyperShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHyperShooterCharacter, WeaponComponent);
	DOREPLIFETIME(AHyperShooterCharacter, StatusComponent);
}

float AHyperShooterCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (HasAuthority())
	{
		if (StatusComponent)
		{
			FHitResult hitResult;
			FVector ImpulseDir;
				
			if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
			{
				FPointDamageEvent* PointDamageEvent = (FPointDamageEvent*)&DamageEvent;
				PointDamageEvent->GetBestHitInfo(this, EventInstigator, hitResult, ImpulseDir);
			}
			else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
			{
				FRadialDamageEvent* RadialDamageEvent = (FRadialDamageEvent*)&DamageEvent;
				RadialDamageEvent->GetBestHitInfo(this, EventInstigator, hitResult, ImpulseDir);
			}
			else
			{
				DamageEvent.GetBestHitInfo(this, EventInstigator, hitResult, ImpulseDir);
			}

			// Apply Damage
			float currentHp = StatusComponent->TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

			// Get Hit Direction
			EHSHitDirection hitDirection = GetHitReactDirection(hitResult.ImpactPoint, DamageCauser);

			// Hit or Died
			if (currentHp <= 0 && !bCharacterDied)
			{
				bCharacterDied = true;
				CharacterDied_Multicast(hitDirection);
			}

			if (!bCharacterDied)
			{
				CharacterDamaged_Multicast(DamageCauser, hitDirection);
			}
		}
	}

	return Damage;
}

void AHyperShooterCharacter::WeaponInfoUpdated()
{
	if (IsLocallyControlled())
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		HUDWidget = CreateWidget<UWidget_HUD>(PC, WeaponComponent->GetHUDClass());
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}

		WeaponComponent->Delegate_OnTargetHit.AddUObject(this, &AHyperShooterCharacter::OnFireHit);
	}
}

void AHyperShooterCharacter::PlayMontage_Multicast_Implementation(UAnimMontage* InMontage)
{
	PlayAnimMontage(InMontage);
}

void AHyperShooterCharacter::UpdateWeaponInfo_Server_Implementation(UHSWeaponData* InWeaponData)
{
	if (WeaponComponent && InWeaponData)
	{
		WeaponComponent->UpdateWeaponInfo_Server(InWeaponData);

		GetMesh()->GetAnimInstance()->LinkAnimClassLayers(InWeaponData->CharacterAnimClass);
		PlayMontage_Multicast(InWeaponData->CharacterEquipMontage);
	}
}

void AHyperShooterCharacter::OnAmmoUpdated(int MaxAmmo, int CurrentAmmo)
{
	if (HUDWidget)
	{
		HUDWidget->UpdateAmmo(MaxAmmo, CurrentAmmo);
	}
}

void AHyperShooterCharacter::DashPressed()
{
	DashPressed_Server();

	if (UCharacterMovementComponent* movementComp = GetCharacterMovement())
	{
		movementComp->MaxWalkSpeed = MaxDashSpeed;
	}
}

void AHyperShooterCharacter::DashReleased()
{
	DashReleased_Server();

	if (UCharacterMovementComponent* movementComp = GetCharacterMovement())
	{
		movementComp->MaxWalkSpeed = MaxWalkSpeed;
	}
}

void AHyperShooterCharacter::DashPressed_Server_Implementation()
{
	if (UCharacterMovementComponent* movementComp = GetCharacterMovement())
	{
		movementComp->MaxWalkSpeed = MaxDashSpeed;
	}
}

void AHyperShooterCharacter::DashReleased_Server_Implementation()
{
	if (UCharacterMovementComponent* movementComp = GetCharacterMovement())
	{
		movementComp->MaxWalkSpeed = MaxWalkSpeed;
	}
}


void AHyperShooterCharacter::CrouchPressed_Server_Implementation()
{
	if (GetCharacterMovement()->IsWalking())
	{
		Crouch();
		CrouchPressed_Multicast();
	}
}

void AHyperShooterCharacter::CrouchReleased_Server_Implementation()
{
	UnCrouch();
	CrouchReleased_Multicast();
}

void AHyperShooterCharacter::CrouchPressed_Multicast_Implementation()
{
	if (UHSAnimInstanceBase* animInst = Cast<UHSAnimInstanceBase>(GetMesh()->GetAnimInstance()))
	{
		animInst->SetCrouchState(true);
	}
}

void AHyperShooterCharacter::CrouchReleased_Multicast_Implementation()
{
	if (UHSAnimInstanceBase* animInst = Cast<UHSAnimInstanceBase>(GetMesh()->GetAnimInstance()))
	{
		animInst->SetCrouchState(false);
	}
}


void AHyperShooterCharacter::DropWeapon_Server_Implementation()
{
	if (WeaponComponent->GetWeaponType() != EWeaponType::Pistol)
	{
		// Drop Weapon
		WeaponComponent->DropWeaponFromOwner();

		// Spawn Default Weapon Again
		UpdateWeaponInfo_Server(DefaultWeaponData);
	}
}

void AHyperShooterCharacter::FirePressed()
{
	if (WeaponComponent->HasEnoughAmmo())
	{
		Fire_Server();

		if (HUDWidget)
		{
			HUDWidget->PlayFireAnimation();
		}
	}
	else
	{
		Reload_Server();
	}

	if (WeaponComponent->CanAutoFire() && !GetWorldTimerManager().IsTimerActive(FireHandle))
	{
		GetWorldTimerManager().SetTimer(FireHandle, this,
			&AHyperShooterCharacter::FirePressed, WeaponComponent->GetFireLatency(), true);
	}
}

void AHyperShooterCharacter::PlayFireAnimationWithWeapon()
{
	PlayAnimMontage(WeaponComponent->GetCharacterFireMontage());

	if (UHSAnimInstanceBase* animInst = Cast<UHSAnimInstanceBase>(GetMesh()->GetAnimInstance()))
	{
		animInst->FireMontagePlayed();
	}

	if (HasAuthority())
	{
		WeaponComponent->Fire_Server();
	}
}

void AHyperShooterCharacter::Fire_Server_Implementation()
{
	if (UHSAnimInstanceBase* animInst = Cast<UHSAnimInstanceBase>(GetMesh()->GetAnimInstance()))
	{
		Fire_Multicast();
	}
}

void AHyperShooterCharacter::Fire_Multicast_Implementation()
{
	PlayFireAnimationWithWeapon();
}

void AHyperShooterCharacter::FireReleased()
{
	FireReleased_Server();
}

void AHyperShooterCharacter::FireReleased_Server_Implementation()
{
	FireReleased_Multicast();
}

void AHyperShooterCharacter::FireReleased_Multicast_Implementation()
{
	GetWorldTimerManager().ClearTimer(FireHandle);
}

void AHyperShooterCharacter::Reload_Server_Implementation()
{
	Reload_Multicast();
}

void AHyperShooterCharacter::Reload_Multicast_Implementation()
{
	if (GetCurrentMontage() != WeaponComponent->GetCharacterReloadMontage())
	{
		PlayAnimMontage(WeaponComponent->GetCharacterReloadMontage());
		WeaponComponent->PlayReloadMontage();
	}
}

void AHyperShooterCharacter::OnFireHit()
{
	if (IsLocallyControlled() && HUDWidget)
	{
		HUDWidget->PlayImpactAnimation();
	}
}

EHSHitDirection AHyperShooterCharacter::GetHitReactDirection(const FVector& ImpactPoint, AActor* DamageCauser) const
{
	if (!DamageCauser) return EHSHitDirection::Front;

	const FVector& ActorLocation = GetActorLocation();
	const FVector& DamageCauseLocation = DamageCauser->GetActorLocation();
	FVector TowardVec = DamageCauseLocation - ActorLocation;
	float DotProductResult = FVector::DotProduct(GetActorForwardVector(), TowardVec);

	// PointPlaneDist is super cheap - 1 vector subtraction, 1 dot product.
	float DistanceToFrontBackPlane = FVector::PointPlaneDist(ImpactPoint, ActorLocation, GetActorRightVector());
	float DistanceToRightLeftPlane = FVector::PointPlaneDist(ImpactPoint, ActorLocation, GetActorForwardVector());


	if (FMath::Abs(DistanceToFrontBackPlane) <= FMath::Abs(DistanceToRightLeftPlane))
	{
		// Determine if Front or Back

		// Can see if it's left or right of Left/Right plane which would determine Front or Back
		if (DotProductResult > 0 || DistanceToRightLeftPlane >= 0)
		{
			return EHSHitDirection::Front;
		}
		else
		{
			return EHSHitDirection::Back;
		}
	}
	else
	{
		// Determine if Right or Left

		if (DistanceToFrontBackPlane >= 0)
		{
			return EHSHitDirection::Right;
		}
		else
		{
			return EHSHitDirection::Left;
		}
	}

	return EHSHitDirection::Front;
}

void AHyperShooterCharacter::CharacterDamaged_Multicast_Implementation(AActor* DamageCauser, EHSHitDirection hitDirection)
{
	PlayHitMontage(hitDirection);

	if (IsLocallyControlled() && HUDWidget) 
	{
		HUDWidget->ShowHitIndicator(DamageCauser->GetActorLocation());
	}
}

void AHyperShooterCharacter::CharacterDied_Multicast_Implementation(EHSHitDirection hitDirection)
{
	OnCharacterDied.Broadcast(this);
	PlayDeathMontage(hitDirection);
}

void AHyperShooterCharacter::ReloadSuccess()
{
	if (HasAuthority())
	{
		WeaponComponent->ReloadSuccess();
	}
}


//////////////////////////////////////////////////////////////////////////
// Input

void AHyperShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHyperShooterCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHyperShooterCharacter::Look);

		// Dash
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AHyperShooterCharacter::DashPressed);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Completed, this, &AHyperShooterCharacter::DashReleased);

		// Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AHyperShooterCharacter::FirePressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AHyperShooterCharacter::FireReleased);

		// Crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AHyperShooterCharacter::CrouchPressed_Server);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AHyperShooterCharacter::CrouchReleased_Server);

		// Drop Weapon
		EnhancedInputComponent->BindAction(DropWeaponAction, ETriggerEvent::Started, this, &AHyperShooterCharacter::DropWeapon_Server);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AHyperShooterCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AHyperShooterCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}