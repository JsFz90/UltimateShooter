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

#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Item.h"
#include "Weapon.h"
#include "Ammo.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
	: BaseTurnRate(45.f), BaseLookUpRate(45.f), bAiming(false),
	CameraDefaultFOV(0.f), CameraZoomedFOV(25.f), CameraCurrentFOV(0.f), ZoomInterpSpeed(30.f),
	HipTurnRate(90.f), HipLookUpRate(90.f), AimingTurnRate(20.f), AimingLookUpRate(20.f),
	MouseHipTurnRate(1.0f), MouseHipLookUpRate(1.0f), MouseAimingTurnRate(0.4f), MouseAimingLookUpRate(0.4f),
	CrosshairSpreadMultiplier(0.f), CrosshairVelocityFactor(0.f), CrosshairInAirFactor(0.f), CrosshairAimFactor(0.f), CrosshairShootingFactor(0.f),
	ShootTimeDuration(0.05f), bFiringBullet(false), bFireButtonPressed(false), bShouldFire(true), AutomaticFireRate(0.1f),
	bShouldTraceForItem(false), CameraInterpDistance(250.f), CameraInterpElevation(65.f), Starting9mmAmmo(85), StartingARAmmo(120),
	CombatState(ECombatState::ECS_Unoccupied), bCrouching(false), BaseMovementSpeed(650.f), CrouchMovementSpeed(300.f),
	StandingCapsuleHeight(88.f), CrouchingCapsuleHeight(44.f), BaseGroundFriction(2.f), CrouchingGroundFriction(100.f),
	bShouldPlayPickupSound(true), bShouldPlayEquipSound(true), PickupSoundResetTime(0.2f), EquipSoundResetTime(0.2f)
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

	// Create Hand Scene Component and not need SetupAttachment 
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComponent"));

	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponInterpComp"));
	WeaponInterpComp->SetupAttachment(GetFollowCamera());

	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent1"));
	InterpComp1->SetupAttachment(GetFollowCamera());
	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent2"));
	InterpComp2->SetupAttachment(GetFollowCamera());
	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent3"));
	InterpComp3->SetupAttachment(GetFollowCamera());
	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent4"));
	InterpComp4->SetupAttachment(GetFollowCamera());
	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent5"));
	InterpComp5->SetupAttachment(GetFollowCamera());
	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent6"));
	InterpComp6->SetupAttachment(GetFollowCamera());
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

	// Spawn the default weapon and attach it to the mesh
	EquipWeapon(SpawnDefaultWeapon());

	InitializeAmmoMap();

	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

	// Create FInterpLocation structs for each interp Locations. Add to Array
	InitializeInterpLocations();
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
	if (!EquippedWeapon) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	
	if (WeaponHasAmmo())
	{
		PlayFireSound();
		SendBullet();
		PlayGunFireMontage();

		StartCrosshairBulletFire();        // Start bullet fire timer for crosshairs
		EquippedWeapon->DecrementAmmo();

		StartFireTimer();
	}
}

void AShooterCharacter::StartFiring()  
{
	bFireButtonPressed = true; // No necesario
	FireWeapon();
}

void AShooterCharacter::StopFiring()
{
	bFireButtonPressed = false;  // No necesario
}

void AShooterCharacter::StartFireTimer()
{
	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
}

void AShooterCharacter::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if (!WeaponHasAmmo()) ReloadWeapon();
}


bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	// Check for crosshair trace hit
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation, 50'000.f);

	if (bCrosshairHit)
	{
		OutBeamLocation = CrosshairHitResult.Location;
	}
	// if no hit - OutBeamLocation is the End location for the line trace

	// Perform a second trace, this time from the gun barrel
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - MuzzleSocketLocation };           //fix
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f };

	FHitResult WeaponTraceHit;
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

	if (WeaponTraceHit.bBlockingHit)  // object between barrel and EndPoint
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;  // true if hit
	}
	
	return false;
}

void AShooterCharacter::Aim()
{
	if (CombatState == ECombatState::ECS_Reloading) return;
	
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AShooterCharacter::StopAiming()
{
	bAiming = false;
	if (!bCrouching) { GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed; }
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

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation, float Distance)
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

	if (bScreenToWorld)
	{
		// Trace from crosshair world location outward
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * Distance };
		OutHitLocation = End;    // Set endpoint to line trace endpoint

		// Trace outward from crosshairs world location
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit) 
		{ 
			OutHitLocation = OutHitResult.Location;
			return true; 
		}
	}

	return false;
}

void AShooterCharacter::TraceForItemsInformation()
{
	if (bShouldTraceForItem)
	{
		FHitResult ItemHitResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemHitResult, HitLocation, 500.f);

		if (ItemHitResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemHitResult.GetActor());
			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
			}

			if (ItemHitLastFrame && (TraceHitItem != ItemHitLastFrame))  // we are hitting a different AItem this frame from the last frame or HitItem is null
			{
				ItemHitLastFrame->GetPickupWidget()->SetVisibility(false);
			}
			ItemHitLastFrame = TraceHitItem;
		}
		else
		{
			if (ItemHitLastFrame) { ItemHitLastFrame->GetPickupWidget()->SetVisibility(false); }
			ItemHitLastFrame = nullptr;
			TraceHitItem = nullptr;
		}
	}
	else
	{
		if (ItemHitLastFrame) ItemHitLastFrame->GetPickupWidget()->SetVisibility(false);
		ItemHitLastFrame = nullptr;
		TraceHitItem = nullptr;
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	// Check the TSubclassOf variable
	if (DefaultWeaponClass)
	{
		// Spawn the weapon
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);             // Weapon setting in blueprint
	}

	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip)
	{
		// Get the hand socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RHandSocket"));
		// Attach the weapon to the hand socket RHandSocket
		if (HandSocket) { HandSocket->AttachActor(WeaponToEquip, GetMesh()); }

		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);

		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
		
		EquippedWeapon = nullptr;
	}
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	//TraceHitItem = nullptr;
	//ItemHitLastFrame = nullptr;
}

void AShooterCharacter::SelectWeapon()
{
	if (!TraceHitItem) return;

	TraceHitItem->StartItemCurve(this);
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (!EquippedWeapon) return false;

	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
	if (FireSound) { UGameplayStatics::PlaySound2D(this, FireSound); }
}

void AShooterCharacter::SendBullet()
{
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

		if (MuzzleFlash) { UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform); }

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);

		if (bBeamEnd)
		{
			if (ImpactParticles) { UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd); }
		}

		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
		if (Beam) { Beam->SetVectorParameter(FName("Target"), BeamEnd); }
	}
}

void AShooterCharacter::PlayGunFireMontage()
{
	// PLay Shoot anim Montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::StartReloading()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (!EquippedWeapon) return;

	// Do we have ammo of the correct type
	if (CarryingAmmo() && !EquippedWeapon->GetClipIsFull())  
	{
		if (bAiming) { StopAiming(); }
		CombatState = ECombatState::ECS_Reloading;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (!ReloadMontage || !AnimInstance) return;
		AnimInstance->Montage_Play(ReloadMontage);
		AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
	}
}

void AShooterCharacter::FinishReloading()
{
	// Update the combat state
	CombatState = ECombatState::ECS_Unoccupied;

	// Update the ammo Map
	if (!EquippedWeapon) return;

	const EAmmoType AmmoType{ EquippedWeapon->GetAmmoType() };

	if (AmmoMap.Contains(AmmoType))
	{
		// Ammount of ammo the character is carrying of the Equipped weapon type
		int32 CarriedAmmo = AmmoMap[AmmoType];
		// Space left in the magazine of EquippedWeapon
		const int32 MagazineEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();

		if (MagazineEmptySpace > CarriedAmmo) 
		{
			// Reload the Magazine with all the ammo we are carrying
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			EquippedWeapon->ReloadAmmo(MagazineEmptySpace);
			CarriedAmmo -= MagazineEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
}

bool AShooterCharacter::CarryingAmmo()
{
	if (!EquippedWeapon) return false;
	
	const EAmmoType AmmoType{ EquippedWeapon->GetAmmoType() };

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}

	return false;
}

void AShooterCharacter::GrabClip()
{
	if (!EquippedWeapon) return;
	if (!HandSceneComponent) return;
	
	// Index for the clip bone
	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("hand_l")));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquippedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
}

void AShooterCharacter::Crouch()
{
	if (GetCharacterMovement()->IsFalling()) return;
	//UE_LOG(LogTemp, Warning, TEXT("bool myBool: %d"), bCrouching);
	bCrouching = !bCrouching;

	if (bCrouching) 
	{ 
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;  
		GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
	}
	else 
	{ 
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed; 
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
}

void AShooterCharacter::Jump()
{
	if (bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
	else
	{
		ACharacter::Jump();
	}

}

void AShooterCharacter::InterpCapsuleHeight(float DeltaTime)
{
	float TargetCapsuleHeight{};
	if (bCrouching){ TargetCapsuleHeight = CrouchingCapsuleHeight; }
	else{ TargetCapsuleHeight = StandingCapsuleHeight;}

	const float InterpHeight{ FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHeight, DeltaTime, 20.f) };

	// Negative value if crouching and positive value if standing
	const float DeltaCapsuleHeight{ InterpHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() }; 
	const FVector MeshOffset{ 0.f, 0.f, -DeltaCapsuleHeight };
	GetMesh()->AddLocalOffset(MeshOffset);

	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHeight); // Set New Capsule Height
}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	// Check to see if Ammo Map contains Ammo's AmmoType
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		// Get Amount of ammo in our AmmoMap for Ammo's Type
		int32 AmmoCount{ AmmoMap[Ammo->GetAmmoType()] };
		AmmoCount += Ammo->GetItemCount();
		// Set the Amount of ammo in the map for this type
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	if (EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		if (EquippedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}
	
	Ammo->Destroy();
}

void AShooterCharacter::InitializeInterpLocations()
{
	FInterpLocation WeaponLocation{ WeaponInterpComp, 0 };
	InterpLocations.Add(WeaponLocation);
	FInterpLocation InterpLocation1{ InterpComp1, 0 };
	InterpLocations.Add(InterpLocation1);
	FInterpLocation InterpLocation2{ InterpComp2, 0 };
	InterpLocations.Add(InterpLocation2);
	FInterpLocation InterpLocation3{ InterpComp3, 0 };
	InterpLocations.Add(InterpLocation3);
	FInterpLocation InterpLocation4{ InterpComp4, 0 };
	InterpLocations.Add(InterpLocation4);
	FInterpLocation InterpLocation5{ InterpComp5, 0 };
	InterpLocations.Add(InterpLocation5);
	FInterpLocation InterpLocation6{ InterpComp6, 0 };
	InterpLocations.Add(InterpLocation6);
}

void AShooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}

void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

int32 AShooterCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;
	for (int32 i = 1; i < InterpLocations.Num(); i++) // loop 1-6  0 is the weapon Location for interp
	{
		if (InterpLocations[i].ItemCount < LowestCount) 
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	}

	return LowestIndex;
}

void AShooterCharacter::IncrementInterpLocationCount(int32 Index, int32 Amount)
{
	if (Amount < -1 || Amount > 1) return;
	
	if (InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}

void AShooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;
	GetWorldTimerManager().SetTimer(PickupSoundTimer, this, &AShooterCharacter::ResetPickupSoundTimer, PickupSoundResetTime);
}

void AShooterCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this, &AShooterCharacter::ResetEquipSoundTimer, EquipSoundResetTime);
}


// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle interpolation for zoom when aiming
	CameraInterpZoom(DeltaTime);  
	
	SetLookRates();

	CalculateCrosshairSpread(DeltaTime);

	TraceForItemsInformation();

	// Interpolate the capsule height based on crouching / standing
	InterpCapsuleHeight(DeltaTime);
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
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		// Fire Weapon
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AShooterCharacter::StartFiring);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacter::StopFiring);
		// Aiming 
		EnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Aim);
		EnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Completed, this, &AShooterCharacter::StopAiming);
		// Select 
		EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Triggered, this, &AShooterCharacter::SelectWeapon); // for test
		// Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AShooterCharacter::StartReloading);
		// Crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Crouch);
	}
}

void AShooterCharacter::SetOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItem = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItem = true;
	}
}

// No Longer needed
/*FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
	const FVector CameraForward{ FollowCamera->GetForwardVector() };

	// FVector(0.f, 0.f, CameraInterpElevation)  up vector
	return CameraWorldLocation + CameraForward * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);  
}*/

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	Item->PlayEquipSound();

	AWeapon* Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		SwapWeapon(Weapon);
	}

	AAmmo* Ammo = Cast<AAmmo>(Item);
	if (Ammo)
	{
		PickupAmmo(Ammo);
	}
}

FInterpLocation AShooterCharacter::GetInterpLocation(int32 index)
{
	if (index <= InterpLocations.Num())
	{
		return InterpLocations[index];
	}
	return FInterpLocation();
}

