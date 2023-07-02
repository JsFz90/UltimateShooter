// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"


UENUM(BlueprintType)
enum class EOffsetState : uint8
{
	EOS_Aiming UMETA(DisplayName = "Aiming"),
	EOS_Hip UMETA(DisplayName = "Hip"),
	EOS_Reloading UMETA(DisplayName = "Reloading"),
	EOS_InAir UMETA(DisplayName = "InAir"),

	EOS_MAX UMETA(DisplayName = "DefaultMAX")
};


/**
 * 
 */
UCLASS()
class ULTIMATESHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UShooterAnimInstance();

protected:
	/**  Handle turning in place variables */
	void TurnInPlace();
	/**  Handle calculations for Leaning  */
	void Lean(float DeltaTime);

public:
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	virtual void NativeInitializeAnimation() override;    // override this function

	// Native update override point. It is usually a good idea to simply gather data in this step and 
	// for the bulk of the work to be done in NativeThreadSafeUpdateAnimation.
	//virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	// Native thread safe update override point. Executed on a worker thread just prior to graph update 
	// for linked anim instances, only called when the hosting node(s) are relevant
	//virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	// Native Post Evaluate override point
	//virtual void NativePostEvaluateAnimation() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* ShooterCharacter;

	/** The speed of the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed{ 0.f };

	/** Whether or not the character is in the air */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir{ false };

	/** Whether or not the character is moving */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating{ false };

	/** Offset Yaw used for strafing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw{0.f};

	/** Offset Yaw the frame before we stop (need for stop) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw{ 0.f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming{ false };
	
	/**  Turn In PLace  */
	/**  Yaw of the Character this frame*/
	float CharacterYaw;
	/**  Yaw of the Character the previous frame*/
	float CharacterYawLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In PLace", meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	// Rotation Curve value current frame
	float RotationCurve;
	// Rotation Curve value last frame
	float RotationCurveLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In PLace", meta = (AllowPrivateAccess = "true"))
	float Pitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In PLace", meta = (AllowPrivateAccess = "true"))
	bool bReloading;

	// Offset State used to determine which Aim offset to use
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In PLace", meta = (AllowPrivateAccess = "true"))
	EOffsetState OffsetState;

	/**  Lean */
	/**  Yaw of the Character this frame*/
	FRotator CharacterRotation;
	/**  Yaw of the Character the previous frame*/
	FRotator CharacterRotationLastFrame;
	// Yaw delta used for leaning
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lean", meta = (AllowPrivateAccess = "true"))
	float YawDelta;
	// Crouching
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crouching", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching;
	// Chance RecoilWeight based on turning in place and aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float RecoilWeight;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bTurningInPlace;
};
