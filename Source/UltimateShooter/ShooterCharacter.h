// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"              //EnhancedInput
#include "ShooterCharacter.generated.h"


UCLASS()
class ULTIMATESHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for move at any direction */
	void Move(const FInputActionValue& Value);

	/** Called for look at any direction */
	void Look(const FInputActionValue& Value);

	/** Called when de Fire Button is pressed */
	void FireWeapon();
	void StartFiring();
	void StopFiring();
	void StartFireTimer();
	UFUNCTION()
	void AutoFireReset();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	/** Set bAiming to true or false with button press */
	void Aiming();
	void StopAiming();
	
	void CameraInterpZoom(float DeltaTime);
	// Set Base turn rate and Base Look up rate based on aiming
	void SetLookRates();   

	void CalculateCrosshairSpread(float DeltaTime);

	void StartCrosshairBulletFire();
	UFUNCTION()
	void FinishCrosshairBulletFire();

	/** Line Traced under the crosshairs */
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation, float Distance);
	/** Trace for Items if OverlappedItemCount > 0 */
	void TraceForItemsInformation();

	// Spawns a default weapon and equips it
	class AWeapon* SpawnDefaultWeapon();
	// Takes a weapon and attaches it to the mesh
	void EquipWeapon(class AWeapon* WeaponToEquip);
	// Detach Weapon and let it fall to the ground
	void DropWeapon();
	// Drops Currently equipped weapon and equips traceHitItem
	void SwapWeapon(class AWeapon* WeaponToSwap);

	void SelectWeapon();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/** Camera Boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	/** Camera that follows the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Base turn rate, in deg/sec. Other Scaling may affect final turn rate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate{ 0 };
	/** Base look up/down rate, in deg/sec. Other Scaling may affect final look up/down rate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate{ 0 };
	
	/** Randomized gunshot sound cue */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;
	
	/** Flash spawned at BarrelSocket */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;
	/** Particles spawned upon bullet impact */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ImpactParticles;
	/** Smoke trail for bullet */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* BeamParticles;
	
	/** Montage for firing the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;

	// True when aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bAiming{ false };

	/** Camera field of view values */
	float CameraDefaultFOV;   // Default camera field of view value  - Set in Begin Play
	float CameraZoomedFOV;    // Field of View value for when zoomed in
	float CameraCurrentFOV;   // Current Field of view this frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;  // Interp speed for zooming when aiming

	/** Aim Look Sensitivity - Turn rates for aimin/not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HipTurnRate; // Turn Rate while not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate; // Look Up rate when not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;     // Turn rate when aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;  // Look Up rate when aiming

	/** Mouse Aim Look Sensitivity - Turn rates for aimin/not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;

	//** Crosshairs spread values */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;  // Determines the spread of the crosshairs
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;    // Velocity component for crosshairs spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;       // In air component for crosshairs spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;         // Aim component for crosshairs spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;     // Shooting component for crosshairs spread
	 
	float ShootTimeDuration;
	bool bFiringBullet;
	FTimerHandle CrooshairShootTimer;

	/** Automatic Guns */
	/** Left mouse button or right console trigger pressed */
	bool bFireButtonPressed;
	/** True when we can fire. False when waiting for the timner */
	bool bShouldFire;
	/** Rate of automatic gun fire */
	float AutomaticFireRate;
	/** Sets a Timer between gunshots */
	FTimerHandle AutoFireTimer;

	/** Trace For Items */
	// True if we should trace for item every frame
	bool bShouldTraceForItem;
	// Numbers of overlapped items
	int8 OverlappedItemCount;
	// The AItem we hit last frame
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	class AItem* ItemHitLastFrame;

	// Weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class AWeapon* EquippedWeapon;

	/** Set this in Blueprints for the default weapon class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	/** The Item currently hit by our trace in trace for Items (could be null) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class AItem* TraceHitItem;

	/** Distance outward from the camera for the interp destination */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpDistance;
	/** Distance upward from the camera for the interp destination */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpElevation;


	/** Configuration to handle Inputs */
	// Mapping Context
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;
	// Move Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;
	// Look Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;
	// Jump Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;
	// Fires Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FireAction;
	// Aiming Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* AimingAction;
	// Select Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SelectAction;

public:
	/** Returns CameraBoom subobject */
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; } 
	/** Returns FollowCamera subobject */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns if Character is Aiming */
	FORCEINLINE bool GetIsAiming() const { return bAiming; }

	UFUNCTION(BlueprintCallable)  // Can not be a FORCEINLINE when we are using UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const { return CrosshairSpreadMultiplier;  } 

	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

	/** Add/Subtracts to/from OverlappedItemCount */
	void SetOverlappedItemCount(int8 Amount);

	FVector GetCameraInterpLocation();

	void GetPickupItem(class AItem* Item);
};
