// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CYSHUDMenu.h"
#include "UI/CYSMainMenuWidget.h"

//ENGINE
#include "Engine/Engine.h"
#include "Widgets/SWeakWidget.h"

void ACYSHUDMenu::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine && GEngine->GameViewport)
	{
		MenuWidget = SNew(CYSMainMenuWidget).OwningHUD(this);
		GEngine->GameViewport->AddViewportWidgetContent(SAssignNew(MenuContainerWidget, SWeakWidget).PossiblyNullContent(MenuWidget.ToSharedRef()));

		if (PlayerOwner)
		{
			PlayerOwner->bShowMouseCursor = true;
			PlayerOwner->SetInputMode(FInputModeUIOnly());
		}
	}
}