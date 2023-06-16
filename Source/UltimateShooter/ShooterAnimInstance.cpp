// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	}
}


void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}
