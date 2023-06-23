// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "Blueprint/UserWidget.h"

AShooterPlayerController::AShooterPlayerController()
{

}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Check our OverlayHUD class
	if (!OverlayHUDClass) return;
	OverlayHUD = CreateWidget<UUserWidget>(this, OverlayHUDClass);
	
	if (!OverlayHUD) return;	
	OverlayHUD->AddToViewport();
	OverlayHUD->SetVisibility(ESlateVisibility::Visible);
	
}
