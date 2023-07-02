// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


UShooterAnimInstance::UShooterAnimInstance()
	: Speed(0.f), bIsInAir(false), bIsAccelerating(false), MovementOffsetYaw(0.f), LastMovementOffsetYaw(0.f), bAiming(false),
	CharacterYaw(0.f), CharacterYawLastFrame(0.f), RootYawOffset(0.f), RotationCurveLastFrame(0.0f), RotationCurve(0.0f), Pitch(0.0f),
	bReloading(false), OffsetState(EOffsetState::EOS_Hip), CharacterRotation(FRotator(0.f)), CharacterRotationLastFrame(FRotator(0.f)), YawDelta(0.f),
	RecoilWeight(1.f), bTurningInPlace(false)
{
}


void UShooterAnimInstance::TurnInPlace()
{
	if (!ShooterCharacter) return;

	Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;

	if (Speed > 0 || bIsInAir)  // Reset
	{
		// Don't want to turn in place, Character is moving
		RootYawOffset = 0.f;
		CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		CharacterYawLastFrame = CharacterYaw;

		RotationCurve = 0.f;
		RotationCurveLastFrame = RotationCurve;
	}
	else
	{
		CharacterYawLastFrame = CharacterYaw;                         // Last Frame Yaw Rotation
		CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;      // Current Yaw Rotation
		const float DeltaYaw{ CharacterYaw - CharacterYawLastFrame }; // DeltaYaw 

		// Root Yaw Offset, updated and clamped to [-180, 180]
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - DeltaYaw);
		// 1.0 if turning, 0.0 if not
		const float Turning{ GetCurveValue(TEXT("Turning")) };
		if (Turning > 0)
		{
			bTurningInPlace = true;
			RotationCurveLastFrame = RotationCurve;                  // Last Frame
			RotationCurve = GetCurveValue(TEXT("RotationCurve"));  // Current Frame
			const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };

			// RootYawOffset > 0 - Turning Left ,  RootYawOffset < 0 - Turning Right
			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

			const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };
			if (ABSRootYawOffset > 90)
			{
				const float YawExcess{ ABSRootYawOffset - 90 };
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
		else
		{
			bTurningInPlace = false;
		}

		//if (GEngine) { GEngine->AddOnScreenDebugMessage(1, -1, FColor::Blue, FString::Printf(TEXT("CharacterYaw: %f"), CharacterYaw)); }
		//if (GEngine) { GEngine->AddOnScreenDebugMessage(2, -1, FColor::Red, FString::Printf(TEXT("RootYawOffset: %f"), RootYawOffset)); }
	}

	// Set RecoilWeight
	if (bTurningInPlace)
	{
		if (bReloading)
		{
			RecoilWeight = 1.f;
		}
		else
		{
			RecoilWeight = 0.f;
		}
	}
	else // Not Turning
	{
		if (bIsCrouching)
		{
			if (bReloading)
			{
				RecoilWeight = 1.f;
			}
			else
			{
				RecoilWeight = 0.1f;
			}
		}
		else
		{
			if (bAiming || bReloading)
			{
				RecoilWeight = 1.f;
			}
			else
			{
				RecoilWeight = 0.5f;
			}
		}
	}
}

void UShooterAnimInstance::Lean(float DeltaTime)
{
	if (!ShooterCharacter) return;
	
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterCharacter->GetActorRotation();

	const FRotator DeltaRotation{ UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame) };

	const float Target{ (float)DeltaRotation.Yaw / DeltaTime };
	const float Interp{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f) };

	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);

	if (GEngine) { GEngine->AddOnScreenDebugMessage(1, -1, FColor::Cyan, FString::Printf(TEXT("YawDelta: %f"), YawDelta)); }
}


void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter == nullptr) { ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner()); }

	if (ShooterCharacter)
	{
		bIsCrouching = ShooterCharacter->GetCrouching();
		bReloading = ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading;

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
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

		if (ShooterCharacter->GetVelocity().Size() > 0.f)
		{
			LastMovementOffsetYaw = MovementOffsetYaw;  // keep this value when velocity = 0 do not update this value
		}

		bAiming = ShooterCharacter->GetIsAiming();
		
		if (bReloading) { OffsetState = EOffsetState::EOS_Reloading; }
		else if(bIsInAir) { OffsetState = EOffsetState::EOS_InAir; }
		else if(bAiming) { OffsetState = EOffsetState::EOS_Aiming; }
		else { OffsetState = EOffsetState::EOS_Hip; }

		// Debug
		//FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), AimRotation.Yaw);
		//FString MovementRotationMessage = FString::Printf(TEXT("Movement Rotation: %f"), MovementRotation.Yaw);
		//FString MovementOffsetMessage = FString::Printf(TEXT("Movement Offset Yaw: %f"), MovementOffset);
		//if (GEngine) { GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, MovementOffsetMessage);}
	}

	TurnInPlace();
	Lean(DeltaTime);
}


void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}


