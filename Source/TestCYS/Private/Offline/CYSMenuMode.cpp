// Fill out your copyright notice in the Description page of Project Settings.


#include "Offline/CYSMenuMode.h"
#include "Player/CYSMenuController.h"
#include "HUD/CYSHUDMenu.h"

ACYSMenuMode::ACYSMenuMode()
{
	PlayerControllerClass = ACYSMenuController::StaticClass();
	HUDClass = ACYSHUDMenu::StaticClass();
}