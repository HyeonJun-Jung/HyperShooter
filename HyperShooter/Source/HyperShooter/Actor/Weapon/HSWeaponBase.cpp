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
	
	CurrentAmmo = MaxAmmo;
	OnRep_Ammo();
}

// Called every frame
void AHSWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RecoilTick(DeltaTime);
}

void AHSWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AHSWeaponBase, MaxAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHSWeaponBase, CurrentAmmo, COND_OwnerOnly);
}

void AHSWeaponBase::InitializeWeaponData()
{
	if (WeaponDataAsset)
	{
		Mesh->SetSkeletalMesh(WeaponDataAsset->Mesh);

		MaxAmmo = WeaponDataAsset->MaxAmmo;
		CurrentAmmo = WeaponDataAsset->MaxAmmo;
		MaxRange = WeaponDataAsset->MaxRange;
		AutoFire = WeaponDataAsset->AutoFire;
		DefaultDamage = WeaponDataAsset->DefaultDamage;
		FireLatency = WeaponDataAsset->FireLatency;

		ShellEjectSystem = WeaponDataAsset->ShellEjectSystem;
		ShellEjectMesh = WeaponDataAsset->ShellEjectMesh;

		MuzzleFlashSystem = WeaponDataAsset->MuzzleFlashSystem;
		MuzzlePosition = WeaponDataAsset->MuzzlePosition;

		TracerSystem = WeaponDataAsset->TracerSystem;

		OnRep_Ammo();
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
	}
}

void AHSWeaponBase::HideInteractiveUI_Implementation(UHSInteractComponent* InteractingComponent)
{
	if (WidgetComponent)
	{
		WidgetComponent->SetVisibility(false);
	}
}

void AHSWeaponBase::UpdateWeaponInfo_Multicast_Implementation(UHSWeaponData* InWeaponDataAsset)
{
	if (InWeaponDataAsset)
	{
		WeaponDataAsset = InWeaponDataAsset;
		InitializeWeaponData();
	}
}

void AHSWeaponBase::UpdateWeaponInfo_Server_Implementation(UHSWeaponData* InWeaponDataAsset)
{
	if (InWeaponDataAsset)
	{
		WeaponDataAsset = InWeaponDataAsset;
		InitializeWeaponData();
	}

	UpdateWeaponInfo_Multicast(InWeaponDataAsset);
}

void AHSWeaponBase::Fire_Server_Implementation()
{
	CurrentAmmo--;
	OnRep_Ammo();

	APawn* owner = Cast<APawn>(GetOwner());
	if (owner)
	{
		FHitResult hitResult; TArray<FHitResult> hitResults_Pawn;
		FVector cameraLocation; FVector cameraForwardVec;
		FVector impactPoint;

		UCameraComponent* camera = owner->GetComponentByClass<UCameraComponent>();
		if (camera)
		{
			cameraLocation = camera->GetComponentLocation();
			cameraForwardVec = camera->GetForwardVector();

			FCollisionQueryParams Params;
			TArray<AActor*> IgnoreActors; IgnoreActors.Add(this); IgnoreActors.Add(GetOwner());
			Params.AddIgnoredActors(IgnoreActors);

			// Line Trace With Camera Forward Vector
			GetWorld()->LineTraceSingleByChannel(hitResult, cameraLocation,
				cameraLocation + cameraForwardVec * MaxRange, ECollisionChannel::ECC_Visibility, Params);

			if (hitResult.bBlockingHit)
			{
				impactPoint = hitResult.ImpactPoint;

				// Apply Damage
				if (hitResult.GetActor() && hitResult.GetActor()->IsA<APawn>())
				{
					GetWorld()->LineTraceMultiByChannel(hitResults_Pawn, cameraLocation,
						cameraLocation + cameraForwardVec * MaxRange, ECollisionChannel::ECC_PhysicsBody, Params);
				
					// Target Hit
					TSet<AActor*> hitActors;
					for (auto& hitResult_Pawn : hitResults_Pawn)
					{
						if (hitResult_Pawn.bBlockingHit)
						{
							AActor* hitActor = hitResult_Pawn.GetActor();

							// Already hitted target
							if (hitActors.Contains(hitActor))
								continue;

							// Check hit Component is Skeletal Mesh 
							if (hitResult_Pawn.GetComponent() && hitResult_Pawn.GetComponent()->IsA<USkeletalMeshComponent>())
							{
								hitActors.Add(hitActor);

								// Check HeadShot
								UE_LOG(LogTemp, Display, TEXT("Hit BoneName : %s"), *hitResult_Pawn.BoneName.ToString());
								if (hitResult_Pawn.BoneName == FName("head") || hitResult_Pawn.BoneName == FName("Head"))
								{
									DefaultDamage *= 4;
								}

								TargetHitSuccessed_Client();

								FVector hitDirection = (owner->GetActorLocation() - hitActor->GetActorLocation());
								hitDirection.Normalize();
								UGameplayStatics::ApplyPointDamage(hitActor, DefaultDamage, hitDirection, hitResult_Pawn,
									owner->GetController(), owner, UDamageType::StaticClass());
							}
						}
					}
				}
				// Spawn Decal
				else
				{
					SpawnBulletHole_Multicast(hitResult.ImpactPoint, hitResult.ImpactNormal);
				}
			}
			else
			{
				impactPoint = cameraLocation + cameraForwardVec * MaxRange;
			}

			// Show Montage and FX
			Fire_Multicast(cameraLocation, impactPoint);
		}
	}
}

void AHSWeaponBase::Fire_Multicast_Implementation(FVector CameraLocation, FVector ImpactPoint)
{
	if (WeaponDataAsset)
	{
		Mesh->GetAnimInstance()->Montage_Play(WeaponDataAsset->FireMontage);
	}

	ApplyRecoil();
	ShowShellEject();
	ShowMuzzleFlash(CameraLocation, ImpactPoint);
	ShowTracer(CameraLocation, ImpactPoint);
}

void AHSWeaponBase::PlayReloadMontage()
{
	if (WeaponDataAsset)
	{
		if (UAnimInstance* animInst = Mesh->GetAnimInstance())
		{
			animInst->Montage_Play(WeaponDataAsset->ReloadMontage);
		}
	}
}

void AHSWeaponBase::ReloadSuccess()
{
	if (HasAuthority())
	{
		CurrentAmmo = MaxAmmo;
		OnRep_Ammo();
	}
}

void AHSWeaponBase::AttachWeaponToCharacter(ACharacter* character)
{
	if (Mesh)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetSimulatePhysics(false);
	}

	if (character)
	{
		AttachToComponent(character->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName("weapon_r"));
		SetActorRelativeRotation(FRotator(0.f, -90.f, 0.f));
		SetActorRelativeLocation(FVector::ZeroVector);
		SetOwner(character);
	}

	if (WidgetComponent)
	{
		WidgetComponent->SetVisibility(false);
	}

	bIsInteractable = false;
}

void AHSWeaponBase::DropWeaponFromOwner()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (Mesh)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetSimulatePhysics(true);
	}

	if (WidgetComponent)
	{
		WidgetComponent->SetVisibility(false);
	}

	bIsInteractable = true;
}

EWeaponType AHSWeaponBase::GetWeaponType() const
{
	if (WeaponDataAsset)
	{
		return WeaponDataAsset->WeaponType;
	}

	return EWeaponType::Pistol;
}

void AHSWeaponBase::RecoilTick(float DeltaTime)
{
	// For Recoil Sync With Server (Using value from curve)
	TimeSinceFire += DeltaTime;
	if (TimeSinceFire >= 0.2f && RecoilIndex != 0)
	{
		RecoilIndex = FMath::FInterpTo(RecoilIndex, 0, DeltaTime, 3.f);
	}

	// Get New Recoil Offset
	FVector2D NewRecoilOffset = FMath::Vector2DInterpTo(
		CurrentRecoilOffset,
		TargetRecoilOffset,
		DeltaTime,
		10.f
	);

	// Calc delta value
	FVector2D Delta = NewRecoilOffset - CurrentRecoilOffset;
	CurrentRecoilOffset = NewRecoilOffset;

	// Apply Recoil To Controller
	APawn* player = Cast<APawn>(GetOwner());
	if (player)
	{
		APlayerController* PC = Cast<APlayerController>(player->GetController());
		if (PC)
		{
			PC->AddPitchInput(-Delta.X);
			PC->AddYawInput(Delta.Y);
		}
	}

	TargetRecoilOffset = FMath::Vector2DInterpTo(
		TargetRecoilOffset,
		FVector2D::ZeroVector,
		DeltaTime,
		5.f
	);
}

void AHSWeaponBase::ApplyRecoil()
{
	TimeSinceFire = 0.f;
	RecoilIndex = (RecoilIndex + 1) % 10;

	APawn* Player = Cast<APawn>(GetOwner());
	if (Player && WeaponDataAsset)
	{
		APlayerController* PC = Cast<APlayerController>(Player->GetController());

		if (WeaponDataAsset->RecoilPitchCurve)
		{
			float Pitch = WeaponDataAsset->RecoilPitchCurve->GetFloatValue(RecoilIndex);
			TargetRecoilOffset.X += Pitch;
			// PC->AddPitchInput(-Pitch);
		}

		if (WeaponDataAsset->RecoilYawCurve)
		{
			float Yaw = WeaponDataAsset->RecoilYawCurve->GetFloatValue(RecoilIndex);
			TargetRecoilOffset.Y += Yaw;
			// PC->AddYawInput(Yaw);
		}
	}
}

void AHSWeaponBase::ShowShellEject()
{
	if (ShellEjectSystem)
	{
		if (!NC_ShellEject || !NC_ShellEject->IsActive())
		{
			FTransform transform = Mesh->GetSocketTransform(FName("ShellEject"), ERelativeTransformSpace::RTS_Actor);

			NC_ShellEject = UNiagaraFunctionLibrary::SpawnSystemAttached(ShellEjectSystem, Mesh, FName(),
				transform.GetLocation(), FRotator(0.f, 90.f, 0.f),
				EAttachLocation::KeepRelativeOffset, true, true);

			NC_ShellEject->SetVariableStaticMesh(FName("User.ShellEjectStaticMesh"), ShellEjectMesh);

			ShellEjectTrigger = false;
		}

		ShellEjectTrigger = !ShellEjectTrigger;
		NC_ShellEject->SetNiagaraVariableBool(FName("User.Trigger").ToString(), ShellEjectTrigger);
	}
}

void AHSWeaponBase::ShowMuzzleFlash(FVector cameraLocation, FVector ImpactPoint)
{
	if (MuzzleFlashSystem)
	{
		if (!NC_MuzzleFlash || !NC_MuzzleFlash->IsActive())
		{
			FTransform transform = Mesh->GetSocketTransform(FName("Muzzle"), ERelativeTransformSpace::RTS_Actor);

			NC_MuzzleFlash = UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlashSystem, Mesh, FName(),
				transform.GetLocation(), FRotator(0.f, 90.f, 0.f),
				EAttachLocation::KeepRelativeOffset, true, true);

			MuzzleFlashTrigger = false;
		}

		MuzzleFlashTrigger = !MuzzleFlashTrigger;
		NC_MuzzleFlash->SetNiagaraVariableBool(FName("User.Trigger").ToString(), MuzzleFlashTrigger);

		FVector direction = ImpactPoint - Mesh->GetSocketLocation(FName("Muzzle"));
		direction.Normalize(0.0001);
		NC_MuzzleFlash->SetNiagaraVariableVec3(FName("User.Direction").ToString(), direction);
	}
}

void AHSWeaponBase::ShowTracer(FVector cameraLocation, FVector ImpactPoint)
{
	if (TracerSystem)
	{
		if (!NC_Tracer || !NC_Tracer->IsActive())
		{
			FTransform transform = Mesh->GetSocketTransform(FName("Muzzle"), ERelativeTransformSpace::RTS_Actor);

			NC_Tracer = UNiagaraFunctionLibrary::SpawnSystemAttached(TracerSystem, Mesh, FName(),
				transform.GetLocation(), FRotator(0.f, 90.f, 0.f),
				EAttachLocation::KeepRelativeOffset, true, true);

			TracerTrigger = false;
		}

		TracerTrigger = !TracerTrigger;
		NC_Tracer->SetNiagaraVariableBool(FName("User.Trigger").ToString(), TracerTrigger);

		TArray<FVector> array; array.Push(ImpactPoint);
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NC_Tracer, FName("User.ImpactPositions"), array);
	}
}

void AHSWeaponBase::OnRep_Ammo()
{
	Delegate_OnAmmoUpdated.Broadcast(MaxAmmo, CurrentAmmo);
}

void AHSWeaponBase::SpawnBulletHole_Multicast_Implementation(FVector ImpactPoint, FVector ImpactNormal)
{
	if (WeaponDataAsset)
	{
		UDecalComponent* decal = UGameplayStatics::SpawnDecalAtLocation(this, WeaponDataAsset->BulletHoleDecalMaterial,
			FVector(5.f, 5.f, 5.f), ImpactPoint,
			UKismetMathLibrary::MakeRotFromX(ImpactNormal), 4.f);

		if (decal)
		{
			decal->SetFadeScreenSize(0.f);
		}
	}
}

void AHSWeaponBase::TargetHitSuccessed_Client_Implementation()
{
	Delegate_OnTargetHit.Broadcast();
}

UAnimMontage* AHSWeaponBase::GetCharacterFireMontage() const
{
	if (WeaponDataAsset)
	{
		return WeaponDataAsset->CharacterFireMontage;
	}
	return nullptr;
}

UAnimMontage* AHSWeaponBase::GetCharacterReloadMontage() const
{
	if (WeaponDataAsset)
	{
		return WeaponDataAsset->CharacterReloadMontage;
	}
	return nullptr;
}