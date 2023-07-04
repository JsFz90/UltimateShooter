// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h" 
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

#include "ShooterCharacter.h"

// Sets default values
AItem::AItem()
	: ItemName(FString("Default")), ItemCount(0), ItemRarity(EItemRarity::EIR_Common), ItemState(EItemState::EIS_Pickup),
	ItemIterpStartLocation(FVector(0.f)), CameraTargetLocation(FVector(0.f)), bInterping(false), ZCurveTime(0.7f),
	ItemInterpX(0.f), ItemInterpY(0.f), InterpInitialYawOffset(0.f), ItemType(EItemType::EIT_MAX), InterpLocationIndex(0)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(ItemMesh);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget")); 
	PickupWidget->SetupAttachment(GetRootComponent());

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	
	// Hide Pickup widget
	if (PickupWidget) { PickupWidget->SetVisibility(false); }
	
	// Set Active stars 
	SetActiveStars();

	// Setup Overlap for Area Sphere
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	// Set Items properties based on ItemState
	SetItemsProperties(ItemState);
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		
		if (ShooterCharacter){ ShooterCharacter->SetOverlappedItemCount(1); }
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);

		if (ShooterCharacter) { ShooterCharacter->SetOverlappedItemCount(-1); }
	}
}

void AItem::SetActiveStars()
{
	// The zero element is not used
	for (int32 i = 0; i <= 5; i++)
	{
		ActiveStars.Add(false);
	}

	switch (ItemRarity)
	{
		case EItemRarity::EIR_Damaged:
		{
			ActiveStars[1] = true;
			break;
		}
		case EItemRarity::EIR_Common:
		{
			ActiveStars[1] = true;
			ActiveStars[2] = true;
			break;
		}
		case EItemRarity::EIR_Uncommon:
		{
			ActiveStars[1] = true;
			ActiveStars[2] = true;
			ActiveStars[3] = true;
			break;
		}
		case EItemRarity::EIR_Rare:
		{
			ActiveStars[1] = true;
			ActiveStars[2] = true;
			ActiveStars[3] = true;
			ActiveStars[4] = true;
			break;
		}
		case EItemRarity::EIR_Legendary:
		{
			ActiveStars[1] = true;
			ActiveStars[2] = true;
			ActiveStars[3] = true;
			ActiveStars[4] = true;
			ActiveStars[5] = true;
			break;
		}
	}
}

void AItem::SetItemsProperties(EItemState State)
{
	switch (State)
	{
		case EItemState::EIS_Pickup:
		{
			// Set Mesh properties
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetVisibility(true);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			// Set Area Sphere properties
			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			// Set Collision Box properties
			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			break;
		}
		case EItemState::EIS_EquipInterping:
		{
			// Set Mesh properties
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetVisibility(true);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			// Set Area Sphere properties
			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			// Set Collision Box properties
			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
		}
		case EItemState::EIS_PickedUp:
		{
			break;
		}
		case EItemState::EIS_Equipped:
		{
			//if (PickupWidget) { PickupWidget->SetVisibility(false); }
			// Set Mesh properties
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetVisibility(true);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			// Set Area Sphere properties
			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			// Set Collision Box properties
			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
		}
		case EItemState::EIS_Falling:
		{
			// Set Mesh properties
			ItemMesh->SetSimulatePhysics(true);
			ItemMesh->SetEnableGravity(true);
			ItemMesh->SetVisibility(true);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			// Set Area Sphere properties
			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			// Set Collision Box properties
			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
		}
	}

}

void AItem::FinishInterping()
{
	bInterping = false;
	if (Character)
	{
		Character->IncrementInterpLocationCount(InterpLocationIndex, -1);
		Character->GetPickupItem(this);
	}
	// Back to intem normal scale
	SetActorScale3D(FVector(1.0f));


}

void AItem::ItemInterp(float DeltaTime)
{
	if (!bInterping) return;

	if (Character && ItemZCurve)
	{
		// Elapsed time since we started ItemInterpTimer
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
		// Get Curve value corresponding to ElapsedTime
		const float CurveValue = ItemZCurve->GetFloatValue(ElapsedTime);

		FVector ItemLocation = ItemIterpStartLocation;
		// Get Location in front of the camera
		//const FVector CameraInterpLocation{ Character->GetCameraInterpLocation() };
		const FVector CameraInterpLocation{ GetInterpLocation()};
		// Vector from Item to Camera InterpLocation, x and y are zero - up vector
		const FVector ItemToCamera{ FVector(0.f, 0.f, (CameraInterpLocation - ItemLocation).Z) };
		// Scale factor to multiply with Curve value
		const float DeltaZ = ItemToCamera.Size();

		// Interp x and y values
		const FVector CurrentLocation{ GetActorLocation() };
		const float InterpXValue = FMath::FInterpTo(CurrentLocation.X, CameraInterpLocation.X, DeltaTime, 30.f);
		const float InterpYValue = FMath::FInterpTo(CurrentLocation.Y, CameraInterpLocation.Y, DeltaTime, 30.f);

		ItemLocation.X = InterpXValue;
		ItemLocation.Y = InterpYValue;

		// Adding curve value to the Z component of the Initial Location (scaled by DeltaZ)
		ItemLocation.Z += CurveValue * DeltaZ;
		SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);

		// Camera and Item rotation this frame
		const FRotator CameraRotation{ Character->GetFollowCamera()->GetComponentRotation() };
		FRotator ItemRotation{ 0.f, CameraRotation.Yaw + InterpInitialYawOffset, 0.f };

		SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

		// Scale
		if (ItemScaleCurve)
		{
			const float ScaleCurveValue = ItemScaleCurve->GetFloatValue(ElapsedTime);
			SetActorScale3D(FVector( ScaleCurveValue, ScaleCurveValue, ScaleCurveValue ));
		}
	}
}

FVector AItem::GetInterpLocation()
{
	if (!Character) return FVector(0.f);

	switch (ItemType)
	{
		case EItemType::EIT_Ammo:
		{
			return Character->GetInterpLocation(InterpLocationIndex).SceneComponent->GetComponentLocation();
			break;
		}
		case EItemType::EIT_Weapon:
		{
			return Character->GetInterpLocation(0).SceneComponent->GetComponentLocation();
			break;
		}
	}
	return FVector(0.f);
}

void AItem::PlayPickupSound()
{
	if (!Character) return;
	
	if (Character->GetShouldPlayPickupSound())
	{
		Character->StartPickupSoundTimer();
		if (PickupSound) { UGameplayStatics::PlaySound2D(this, PickupSound); }
	}
}

void AItem::PlayEquipSound()
{
	if (!Character) return;

	if (Character->GetShouldPlayEquipSound())
	{
		Character->StartEquipSoundTimer();
		if (PickupSound) { UGameplayStatics::PlaySound2D(this, PickupSound); }
	}
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ItemInterp(DeltaTime);
}

void AItem::StartItemCurve(AShooterCharacter* ShooterCharacter)
{
	bInterping = true;
	SetItemState(EItemState::EIS_EquipInterping);
	// Store a ref to Character
	Character = ShooterCharacter;
	InterpLocationIndex = Character->GetInterpLocationIndex();
	// Add one to the item Count for this interp Location struct
	Character->IncrementInterpLocationCount(InterpLocationIndex, 1);

	PlayPickupSound();

	// Store Initial Location of the item
	ItemIterpStartLocation = GetActorLocation();

	GetWorldTimerManager().SetTimer(ItemInterpTimer, this, &AItem::FinishInterping, ZCurveTime);

	// Get Initial Yaw of the Camera and the item
	const float CameraRotationYaw{ (float)Character->GetFollowCamera()->GetComponentRotation().Yaw };
	const float ItemRotaionYaw{ (float)GetActorRotation().Yaw };

	// Set Yaw Offset (Delta Yaw) - Initial Yaw offset between Camera and Item
	InterpInitialYawOffset = ItemRotaionYaw - CameraRotationYaw;

}

