// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CYSHUDGame.h"
#include "UI/CYSGameWidget.h"
#include "Player/CYSPlayerController.h"

//ENGINE
#include "Engine/Engine.h"
#include "Widgets/SWeakWidget.h"

void ACYSHUDGame::BeginPlay()
{
	Super::BeginPlay();

}

void ACYSHUDGame::ShowDamage()
{
	if (GEngine && GEngine->GameViewport)
	{
		MenuWidget = SNew(CYSGameWidget).OwningHUD(this);
		GEngine->GameViewport->AddViewportWidgetContent(SAssignNew(MenuContainerWidget, SWeakWidget).PossiblyNullContent(MenuWidget.ToSharedRef()));
	}
}

void ACYSHUDGame::HiddenDamage()
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(MenuContainerWidget.ToSharedRef());
	}
}
