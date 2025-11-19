// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CameraMode.generated.h"

class UCameraModeManagerComponent;
class USpringArmComponent;
class UCameraComponent;

UCLASS(Abstract, Blueprintable, EditInlineNew)
class HYPERSHOOTER_API UCameraMode : public UObject
{
    GENERATED_BODY()

public:
    virtual void OnEnter(class UHSCameraModeComponent* Manager);
    virtual void OnExit();
    virtual void Tick(float DeltaTime);

protected:
    UPROPERTY()
    UHSCameraModeComponent* CameraManager;

protected:
    UPROPERTY(EditAnywhere)
    float TargetArmLength = 300.f;

    UPROPERTY(EditAnywhere)
    FVector TargetOffset;

    UPROPERTY(EditAnywhere)
    FRotator TargetRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere)
    bool bUseControllerDesiredRotation = true;

    UPROPERTY(EditAnywhere)
    bool bOrientRotationToMovement = false;

private:
    UPROPERTY()
    APawn* Player;

    UPROPERTY()
    APlayerController* PC;

    UPROPERTY()
    USpringArmComponent* SpringArm;

    UPROPERTY()
    UCameraComponent* Camera;

    bool bShouldLerp = false;
};
