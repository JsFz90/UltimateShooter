// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "EnhancedInputSubsystems.h"  //EnhancedInput
#include "EnhancedInputComponent.h"   //EnhancedInput

#include "Kismet/GameplayStatics.h"    // Sounds
#include "Sound/SoundCue.h"            // Sounds
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"          // Debug
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter() 
	: BaseTurnRate(45.f), BaseLookUpRate(45.f), bAiming(false), 
	CameraDefaultFOV(0.f), CameraZoomedFOV(35.f), CameraCurrentFOV(0.f), ZoomInterpSpeed(30.f),
	HipTurnRate(90.f), HipLookUpRate(90.f), AimingTurnRate(20.f), AimingLookUpRate(20.f),
	MouseHipTurnRate(1.0f), MouseHipLookUpRate(1.0f), MouseAimingTurnRate(0.2f), MouseAimingLookUpRate(0.2f),
	CrosshairSpreadMultiplier(0.f), CrosshairVelocityFactor(0.f), CrosshairInAirFactor(0.f), CrosshairAimFactor(0.f), CrosshairShootingFactor(0.f),
	ShootTimeDuration(0.05f), bFiringBullet(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/** Create a camera boom  (pulls in towards the character if there is a collision) */
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 250.f;         // Camera fallows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true;   // Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	/** Create a follow camera  */
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);  // Attach camera to end of boom
	FollowCamera->bUsePawnControlRotation = false;                               // Camera does not rotate relative to arm

	// Strafing Movement
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;  // Character moves in the direction of input false 
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);  // at this rotation rate - Yaw Direction (Z)
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	if (FollowCamera)
	{ 
		CameraDefaultFOV = GetFollowCamera()->FieldOfView; 
		CameraCurrentFOV = CameraDefaultFOV;
	}

}

void AShooterCharacter::Move(const FInputActionValue& Value)
{
	// Input is a Vector2D
	FVector2D MovementVector{ Value.Get<FVector2D>() };
	
	if (GetController() && (MovementVector.Y != 0 || MovementVector.X != 0))
	{
		// Add movement 
		//AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		//AddMovementInput(GetActorRightVector(), MovementVector.X);

		/** This is the best way  */
		const FRotator Rotation{ GetController()->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };
		
		const FVector ForwardDirection{ FRotationMatrix{ YawRotation }.GetUnitAxis(EAxis::X)};
		const FVector RightDirection{ FRotationMatrix{ YawRotation }.GetUnitAxis(EAxis::Y) };
		
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AShooterCharacter::Look(const FInputActionValue& Value)
{
	// Input is a Vector2D
	FVector2D LookAxisVector{ Value.Get<FVector2D>() };

	if (GetController() && (LookAxisVector.Y != 0 || LookAxisVector.X != 0))
	{
		// add yaw and pitch input to controller 
		//AddControllerPitchInput(LookAxisVector.Y * BaseLookUpRate * GetWorld()->GetDeltaSeconds()); // Up/Down (Y axis)     deg/sec * sec/frame = deg/frame
		//AddControllerYawInput(LookAxisVector.X * BaseTurnRate * GetWorld()->GetDeltaSeconds());     // Left/Right (Z axis)  deg/sec * sec/frame = deg/frame

		// Mouse 
		float MouseTurnScaleFactor{ 0.f };
		float MouseLookUpScaleFactor{ 0.f };
		
		if (bAiming)
		{
			MouseTurnScaleFactor = MouseAimingTurnRate;
			MouseLookUpScaleFactor = MouseAimingLookUpRate;

		}
		else
		{
			MouseTurnScaleFactor = MouseHipTurnRate;
			MouseLookUpScaleFactor = MouseHipLookUpRate;
		}
		AddControllerPitchInput(LookAxisVector.Y * MouseLookUpScaleFactor);
		AddControllerYawInput(LookAxisVector.X * MouseTurnScaleFactor);
	}
}

void AShooterCharacter::FireWeapon()
{
	if (FireSound){ UGameplayStatics::PlaySound2D(this, FireSound); }

	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (MuzzleFlash) { UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform); }

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);

		if (bBeamEnd)
		{
			if (ImpactParticles) { UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd); }

			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if (Beam) { Beam->SetVectorParameter(FName("Target"), BeamEnd); }
		}
	}

	// PLay Shoot anim Montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

	// Start bullet fire timer for crosshairs
	StartCrosshairBulletFire();
}


bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	/** (Shoot using Crosshair) Get Current Size of Viewport */
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport) { GEngine->GameViewport->GetViewportSize(ViewportSize); }

	/** Get screen space location of crosshairs  */
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	// Get world position and Direction of crosshairs
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld)  // was Deprojection successful?
	{
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f };

		OutBeamLocation = End; // Set Beam endpoint to line trace endpoint

		// Trace outward from crosshairs world location
		FHitResult ScreenTraceHit;
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

		if (ScreenTraceHit.bBlockingHit)  // was there a trace hit?
		{
			OutBeamLocation = ScreenTraceHit.Location;  // Beam endpoint is now trace hit location
		}

		// Perform a second trace, this time from the gun barrel
		const FVector WeaponTraceStart{ MuzzleSocketLocation };
		const FVector WeaponTraceEnd{ OutBeamLocation };

		FHitResult WeaponTraceHit;
		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

		if (WeaponTraceHit.bBlockingHit)  // object between barrel and EndPoint
		{
			OutBeamLocation = WeaponTraceHit.Location;	
		}
		return true;
	}

	return false;
}

void AShooterCharacter::Aiming()
{
	bAiming = true;
}

void AShooterCharacter::StopAiming()
{
	bAiming = false;
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	// Set Current Camera field of View
	if (bAiming) { CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed); } // Interpolate to zoomed FOV
	else { CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed); }        // Interpolate to Default FOV

	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookRates()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	// Calculate crosshair velocity factor
	FVector2D WalkSpeedRange{ 0.f, 600.f };
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.f;
	
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	// Calculate crosshair in air factor
	if (GetCharacterMovement()->IsFalling())
	{
		// Spread the crosshair slowly while in air
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		// Shrink the crosshair rapidly while on the ground
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}

	// Calculeate crosshair aim factor
	if (bAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.6f, DeltaTime, 30.f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	// True 0.05 second after firing
	if (bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);
	}

	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;  // 0.5f Base Spread

}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrooshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}


// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle interpolation for zoom when aiming
	CameraInterpZoom(DeltaTime);  
	
	SetLookRates();

	CalculateCrosshairSpread(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Move
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Move);
		// Look 
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Look);
		// Jump
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		// Fire Weapon
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AShooterCharacter::FireWeapon);
		// Aiming 
		EnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Aiming);
		EnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Completed, this, &AShooterCharacter::StopAiming);
	}
}

