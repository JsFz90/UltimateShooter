// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMATESHOOTER_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	

public:
	AShooterPlayerController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Reference to the Overall HUD Overlay
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> OverlayHUDClass;
	// Variable to hold the HUD Overlay after creating it
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	class UUserWidget* OverlayHUD;
};
