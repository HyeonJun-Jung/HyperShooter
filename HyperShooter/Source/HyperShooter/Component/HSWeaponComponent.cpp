// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/HSWeaponComponent.h"
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


// Sets default values for this component's properties
UHSWeaponComponent::UHSWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("Mesh"));
	Mesh->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void UHSWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentAmmo = MaxAmmo;
	OnRep_Ammo();
}

void UHSWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RecoilTick(DeltaTime);
}

void UHSWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UHSWeaponComponent, MaxAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UHSWeaponComponent, CurrentAmmo, COND_OwnerOnly);
}

void UHSWeaponComponent::InitializeWeaponData()
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

void UHSWeaponComponent::UpdateWeaponInfo_Multicast_Implementation(UHSWeaponData* InWeaponDataAsset)
{
	if (InWeaponDataAsset)
	{
		WeaponDataAsset = InWeaponDataAsset;
		InitializeWeaponData();
	}
}

void UHSWeaponComponent::UpdateWeaponInfo_Server_Implementation(UHSWeaponData* InWeaponDataAsset)
{
	if (InWeaponDataAsset)
	{
		WeaponDataAsset = InWeaponDataAsset;
		InitializeWeaponData();
	}

	UpdateWeaponInfo_Multicast(InWeaponDataAsset);
}

void UHSWeaponComponent::Fire_Server_Implementation()
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
			TArray<AActor*> IgnoreActors; IgnoreActors.Add(GetOwner());
			Params.AddIgnoredActors(IgnoreActors);
			Params.AddIgnoredComponent(Mesh);

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

void UHSWeaponComponent::Fire_Multicast_Implementation(FVector CameraLocation, FVector ImpactPoint)
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

void UHSWeaponComponent::PlayReloadMontage()
{
	if (WeaponDataAsset)
	{
		if (UAnimInstance* animInst = Mesh->GetAnimInstance())
		{
			animInst->Montage_Play(WeaponDataAsset->ReloadMontage);
		}
	}
}

void UHSWeaponComponent::ReloadSuccess()
{
	if (GetOwner()->HasAuthority())
	{
		CurrentAmmo = MaxAmmo;
		OnRep_Ammo();
	}
}

void UHSWeaponComponent::AttachWeaponToCharacter(ACharacter* character)
{
	if (Mesh)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetSimulatePhysics(false);
	}

	if (character)
	{
		AttachToComponent(character->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName("weapon_r"));
		SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
		SetRelativeLocation(FVector::ZeroVector);
	}
}

void UHSWeaponComponent::DropWeaponFromOwner()
{

}

EWeaponType UHSWeaponComponent::GetWeaponType() const
{
	if (WeaponDataAsset)
	{
		return WeaponDataAsset->WeaponType;
	}

	return EWeaponType::Pistol;
}

void UHSWeaponComponent::RecoilTick(float DeltaTime)
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

void UHSWeaponComponent::ApplyRecoil()
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

void UHSWeaponComponent::ShowShellEject()
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

void UHSWeaponComponent::ShowMuzzleFlash(FVector cameraLocation, FVector ImpactPoint)
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

void UHSWeaponComponent::ShowTracer(FVector cameraLocation, FVector ImpactPoint)
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

void UHSWeaponComponent::OnRep_Ammo()
{
	Delegate_OnAmmoUpdated.Broadcast(MaxAmmo, CurrentAmmo);
}

void UHSWeaponComponent::SpawnBulletHole_Multicast_Implementation(FVector ImpactPoint, FVector ImpactNormal)
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

void UHSWeaponComponent::TargetHitSuccessed_Client_Implementation()
{
	Delegate_OnTargetHit.Broadcast();
}

UAnimMontage* UHSWeaponComponent::GetCharacterFireMontage() const
{
	if (WeaponDataAsset)
	{
		return WeaponDataAsset->CharacterFireMontage;
	}
	return nullptr;
}

UAnimMontage* UHSWeaponComponent::GetCharacterReloadMontage() const
{
	if (WeaponDataAsset)
	{
		return WeaponDataAsset->CharacterReloadMontage;
	}
	return nullptr;
}