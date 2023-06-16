// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter == nullptr) { ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner()); }

	if (ShooterCharacter)
	{
		// Get the lateral Speed of the character from velocity
		FVector Velocity{ ShooterCharacter->GetVelocity() };
		Velocity.Z = 0;                                       // Only need x and y (ground velocity)
		Speed = Velocity.Size();

		// Is the Character in the Air?
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

		// Is the Character accelerating?
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f){ bIsAccelerating = true;}
		else{ bIsAccelerating = false;}

		// Strafing Movement
		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
		MovementOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

		// Debug
		//FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), AimRotation.Yaw);
		//FString MovementRotationMessage = FString::Printf(TEXT("Movement Rotation: %f"), MovementRotation.Yaw);
		//FString MovementOffsetMessage = FString::Printf(TEXT("Movement Offset Yaw: %f"), MovementOffset);
		//if (GEngine) { GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, MovementOffsetMessage);}
	}
}


void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}
