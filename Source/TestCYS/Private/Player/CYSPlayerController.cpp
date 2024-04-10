// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CYSPlayerController.h"
#include "HUD/CYSHUDGame.h"

void ACYSPlayerController::ShowDamage()
{
	if (ACYSHUDGame* Hud = Cast<ACYSHUDGame>(GetHUD()))
	{
		Hud->ShowDamage();
	}
}

void ACYSPlayerController::HiddenDamage()
{
	if (ACYSHUDGame* Hud = Cast<ACYSHUDGame>(GetHUD()))
	{
		Hud->HiddenDamage();
	}
}