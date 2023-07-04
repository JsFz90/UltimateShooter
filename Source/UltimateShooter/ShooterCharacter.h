// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"              //EnhancedInput
#include "AmmoType.h"
#include "ShooterCharacter.generated.h"



UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};


USTRUCT(BlueprintType)
struct FInterpLocation
{
	GENERATED_BODY()

	// Scene Component to use for its location for interping
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneComponent;

	// Number of items interping to/ at this scene comp location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemCount;
};


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
	void Aim();
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

	// Initialize the ammo Map with ammo values
	void InitializeAmmoMap();

	// Check to make sure our weapon has ammo;
	bool WeaponHasAmmo();

	// Fire Weapon functions
	void PlayFireSound();
	void SendBullet();
	void PlayGunFireMontage();

	// Reload Weapons functions
	void StartReloading();
	void ReloadWeapon();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	// Check to see if we have ammo of the Equipped Weapon ammo type
	bool CarryingAmmo();

	// Called from animation Blueprint with Grab/Release notify
	UFUNCTION(BlueprintCallable)
	void GrabClip();
	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	void Crouch();

	virtual void Jump() override;

	// Interps capsule half height when crouching / standing
	void InterpCapsuleHeight(float DeltaTime);

	void PickupAmmo(class AAmmo* Ammo);
	
	void InitializeInterpLocations();

	void ResetPickupSoundTimer();
	void ResetEquipSoundTimer();

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CameraDefaultFOV;   // Default camera field of view value  - Set in Begin Play
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
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

	// Map to keep track of ammo of the different types
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap;
	// Starting ammount of 9mm ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmo;
	// Starting ammount of AR ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState;

	// Montage for reloading animation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* ReloadMontage;
	
	// Transform of the clipwhen we first grab the clip during reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FTransform ClipTransform;
	// Scene Component to Attach to the character's hand during reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USceneComponent* HandSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bCrouching;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float BaseMovementSpeed;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float CrouchMovementSpeed;

	// Current Half Height of the capsule
	float CurrentCapsuleHeight{ 0.f };
	// Half Height when not crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float StandingCapsuleHeight;
	// Half Height when crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingCapsuleHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseGroundFriction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingGroundFriction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponInterpComp;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp3;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp4;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp5;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp6;
	// Array of interp location structs
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FInterpLocation> InterpLocations;

	FTimerHandle PickupSoundTimer;
	FTimerHandle EquipSoundTimer;

	bool bShouldPlayPickupSound;
	bool bShouldPlayEquipSound;

	// Time to wait before we can play another sound
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float PickupSoundResetTime;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float EquipSoundResetTime;

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
	// Reload Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ReloadAction;
	// Crouch Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* CrouchAction;

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

	// No Longer needed
	//FVector GetCameraInterpLocation();

	void GetPickupItem(class AItem* Item);

	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }
	FORCEINLINE bool GetCrouching() const { return bCrouching;  }

	FInterpLocation GetInterpLocation(int32 index);

	int32 GetInterpLocationIndex();

	void IncrementInterpLocationCount(int32 Index, int32 Amount);

	FORCEINLINE bool GetShouldPlayPickupSound() const { return bShouldPlayPickupSound; }
	FORCEINLINE bool GetShouldPlayEquipSound() const { return bShouldPlayEquipSound; }

	void StartPickupSoundTimer();
	void StartEquipSoundTimer();
};
