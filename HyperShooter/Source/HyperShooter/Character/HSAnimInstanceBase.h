// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HSAnimInstanceBase.generated.h"

UCLASS()
class HYPERSHOOTER_API UHSAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UHSAnimInstanceBase();

public:
	virtual void NativeInitializeAnimation();
	virtual void NativeUpdateAnimation(float DeltaSeconds);

protected:
	UFUNCTION()
	void AnimNotify_ReloadSuccess();

	UFUNCTION()
	void AnimNotify_ThrowGrenade();

	UFUNCTION()
	void AnimNotify_GrenadeEnd();

protected:
	void UpdatePitchOffset();
	void UpdateYawOffset();
	void UpdateTurnInPlace();

protected:
	void UpdateJumpInfo();
	
public:
	void FireMontagePlayed();
	void SetCrouchState(bool InCrouchingState) { bIsCrouching = InCrouchingState; }

protected:
	void UpdateBlendWeightInfo(float DeltaTime);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Movement")
	bool bShouldMove = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Movement")
	bool bIsCrouching = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Movement")
	float GroundSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Movement")
	float Angle = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Movement")
	float AngleLastTick = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Movement")
	float PitchOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Movement")
	FName YawCurveName = FName("TurnYawWeight");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Movement")
	float RootYawOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Movement")
	float ActualYaw = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Movement")
	float MaxTurnAngle = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Movement")
	float Yaw = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Movement")
	float YawLastTick = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Movement")
	FName DistanceCurveName = FName("RemainingTurnYaw");

	UPROPERTY()
	float DistanceCurveValue = 0.f;

	UPROPERTY()
	float DistanceCurveValueLastTick = 0.f;

	UPROPERTY(BlueprintReadWrite, category = "BlendWeight")
	float HipFireUpperBodyOverrideWieght = 0.f;

	UPROPERTY(BlueprintReadWrite, category = "BlendWeight")
	float AimOffsetBlendWeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "BlendWeight")
	float RaiseWeaponAfterFireDuration = 0.5f;

	UPROPERTY()
	float TimeSinceFireWeapon = 0.f;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Movement")
	bool bIsJumping; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Movement")
	bool bisFalling;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Movement")
	float TimeToJumpApex = 0.f;

protected:
	UPROPERTY(BlueprintReadOnly, category = "Movement")
	class UCharacterMovementComponent* MovementComp;

	UPROPERTY(BlueprintReadOnly, category = "Character")
	class ACharacter* Character;

};
