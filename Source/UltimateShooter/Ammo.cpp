// Fill out your copyright notice in the Description page of Project Settings.


#include "Ammo.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"

#include "ShooterCharacter.h"

AAmmo::AAmmo()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	// Construct the ammo Mesh component and set it as the root
	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
	SetRootComponent(AmmoMesh);

	GetCollisionBox()->SetupAttachment(GetRootComponent());
	GetPickupWidget()->SetupAttachment(GetRootComponent());
	GetAreaSphere()->SetupAttachment(GetRootComponent());

	AmmoCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AmmoCollisionSphere"));
	AmmoCollisionSphere->SetupAttachment(GetRootComponent());
	AmmoCollisionSphere->SetSphereRadius(50.f);
}

void AAmmo::BeginPlay()
{
	Super::BeginPlay();

	AmmoCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAmmo::OnAmmoSphereOverlap);
}

void AAmmo::SetItemsProperties(EItemState State)
{
	Super::SetItemsProperties(State);

	switch (State)
	{
		case EItemState::EIS_Pickup:
		{
			// Set Mesh properties
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
		}
		case EItemState::EIS_EquipInterping:
		{
			// Set Mesh properties
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
		}
		case EItemState::EIS_Falling:
		{
			// Set Mesh properties
			AmmoMesh->SetSimulatePhysics(true);
			AmmoMesh->SetEnableGravity(true);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			break;
		}
	}
}

void AAmmo::OnAmmoSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	
	AShooterCharacter* OverlappedCharacter = Cast<AShooterCharacter>(OtherActor);
	if (OverlappedCharacter)
	{
		StartItemCurve(OverlappedCharacter);
		AmmoCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
}

void AAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
