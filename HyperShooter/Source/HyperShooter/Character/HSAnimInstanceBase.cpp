// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/HSAnimInstanceBase.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HyperShooterCharacter.h"

UHSAnimInstanceBase::UHSAnimInstanceBase()
{
}

void UHSAnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<ACharacter>(TryGetPawnOwner());
	if (IsValid(Character))
		MovementComp = Character->GetCharacterMovement();
}

void UHSAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (MovementComp && Character)
	{
		GroundSpeed = Character->GetVelocity().Size2D();
		FVector vRotation = UKismetMathLibrary::InverseTransformDirection(Character->GetActorTransform(), Character->GetVelocity());
		FRotator rotation = vRotation.Rotation();
		// Angle = rotation.Yaw;

		if (GroundSpeed > 3.f)
			AngleLastTick = FMath::Lerp(AngleLastTick, Angle, 0.5);
		Angle = CalculateDirection(MovementComp->Velocity, Character->GetActorRotation());

		bShouldMove = UKismetMathLibrary::NotEqual_VectorVector(MovementComp->GetCurrentAcceleration(), FVector(0.f, 0.f, 0.f)) && (GroundSpeed > 3.f);
	}

	UpdatePitchOffset();
	UpdateYawOffset();
	UpdateTurnInPlace();

	UpdateJumpInfo();

	UpdateBlendWeightInfo(DeltaSeconds);

	ActualYaw = -RootYawOffset;
}

void UHSAnimInstanceBase::AnimNotify_ReloadSuccess()
{
	if (AHyperShooterCharacter* HSCharacter = Cast<AHyperShooterCharacter>(Character))
	{
		HSCharacter->ReloadSuccess();
	}
}

void UHSAnimInstanceBase::UpdatePitchOffset()
{
	if (Character)
	{
		PitchOffset = UKismetMathLibrary::NormalizedDeltaRotator(Character->GetBaseAimRotation(), Character->GetActorRotation()).Pitch;
	}
}

void UHSAnimInstanceBase::UpdateYawOffset()
{
	if (Character)
	{
		YawLastTick = Yaw;
		Yaw = Character->GetActorRotation().Yaw;

		if (GroundSpeed > 0.f)
		{
			RootYawOffset = 0.f;
			return;
		}

		float YawChangeOverFrame = YawLastTick - Yaw;
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset + YawChangeOverFrame);
	}
}

void UHSAnimInstanceBase::UpdateTurnInPlace()
{
	if (GetCurveValue(FName("TurnYawWeight")))
	{
		float direction = 0;
		(RootYawOffset > 0.f) ? direction = -1.f : direction = 1.f;

		DistanceCurveValueLastTick = DistanceCurveValue;
		DistanceCurveValue = GetCurveValue(DistanceCurveName);

		RootYawOffset = RootYawOffset - (DistanceCurveValueLastTick - DistanceCurveValue) * direction;

		float absRootYawOffset = UKismetMathLibrary::Abs(RootYawOffset);
		if (absRootYawOffset > MaxTurnAngle)
		{
			float yawToSubtract = absRootYawOffset - MaxTurnAngle;
			float yatMultiplier = RootYawOffset > 0.f ? 1.f : -1.f;
			RootYawOffset = RootYawOffset - yawToSubtract * yatMultiplier;
		}
	}
}

void UHSAnimInstanceBase::UpdateJumpInfo()
{
	bIsJumping = false; bisFalling = false;

	if (MovementComp && MovementComp->Velocity.Z != 0.f)
	{
		bIsJumping = MovementComp->Velocity.Z > 0.f ? true : false;
		bisFalling = MovementComp->Velocity.Z > 0.f ? false : true;

		if (bIsJumping)
		{
			float revVelocityZ = 0 - MovementComp->Velocity.Z;
			TimeToJumpApex = revVelocityZ / MovementComp->GetGravityZ();
		}
		else
		{
			TimeToJumpApex = 0.f;
		}
	}
	else
	{
		TimeToJumpApex = 0.f;
	}
}

void UHSAnimInstanceBase::FireMontagePlayed()
{
	TimeSinceFireWeapon = 0.f;
}

void UHSAnimInstanceBase::UpdateBlendWeightInfo(float DeltaTime)
{
	//  TimeSinceFireWeapon < RaiseWeaponAfterFireDuration   || ( ADS && (Crouch || On Air) )
	TimeSinceFireWeapon += DeltaTime;

	if (TimeSinceFireWeapon < RaiseWeaponAfterFireDuration)
	{
		HipFireUpperBodyOverrideWieght = 1.f; 
		AimOffsetBlendWeight = 1.f;
	}
	else
	{
		HipFireUpperBodyOverrideWieght = FMath::FInterpTo(HipFireUpperBodyOverrideWieght, 0.f, DeltaTime, 1.f);
		
		// if(FMath::Abs(RootYawOffset) < 10.f && HasAcce
	}
}
