// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Ammo.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMATESHOOTER_API AAmmo : public AItem
{
	GENERATED_BODY()

public:
	AAmmo();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void SetItemsProperties(EItemState State) override;

	// We need all this parameters when we are using functions with AddDynamic
	UFUNCTION()
	void OnAmmoSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	/** Mesh for ammo pickup */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* AmmoMesh;

	/** Ammo Type for ammo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	UTexture2D* AmmoIconTexture;

	/** Enables pick up ammo when overlap */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AmmoCollisionSphere;

public:
	FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return AmmoMesh; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
};
