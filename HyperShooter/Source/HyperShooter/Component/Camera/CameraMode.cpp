// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Camera/CameraMode.h"
#include "Component/HSCameraModeComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"

void UCameraMode::OnEnter(UHSCameraModeComponent* Manager)
{
    CameraManager = Manager;

    Player = Cast<APawn>(Manager->GetOwner());
    if (Player)
    {
        SpringArm = Player->GetComponentByClass<USpringArmComponent>();
        Camera = Player->GetComponentByClass<UCameraComponent>();
    }

    PC = Cast<APlayerController>(Player->GetController());
    if (PC)
    {
        bShouldLerp = true;
    }
}

void UCameraMode::OnExit()
{
}


void UCameraMode::Tick(float DeltaTime)
{
    if (bShouldLerp && Camera && SpringArm)
    {
        SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetArmLength, DeltaTime, 5.f);

        FRotator TargetControlRotation = Player->GetActorRotation();
        // PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(0, -90, 0), DeltaTime, 2.5f));
        // PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), TargetControlRotation, DeltaTime, 5.f));

        Camera->SetRelativeRotation(FMath::RInterpTo(Camera->GetRelativeRotation(), TargetRotation, DeltaTime, 5.f));
        Camera->SetRelativeLocation(FMath::VInterpTo(Camera->GetRelativeLocation(), TargetOffset, DeltaTime, 5.f));

        if (Camera->GetRelativeRotation().Equals(TargetRotation, 1.0f)
            && Camera->GetRelativeLocation().Equals(TargetOffset, 1.0f)
            && abs(SpringArm->TargetArmLength - TargetArmLength) < 2.5f)
        {
            bShouldLerp = false;
        }
    }
}
