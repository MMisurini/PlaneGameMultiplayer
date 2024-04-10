// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CYSHUDGame.generated.h"

UCLASS()
class TESTCYS_API ACYSHUDGame : public AHUD
{
	GENERATED_BODY()
	
////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS

public:
	virtual void BeginPlay() override;

	void ShowDamage();
	void HiddenDamage();

////////////////////////////////////////////////////////////////////////////////////////////////
// VARIABLES

protected:
	TSharedPtr<class CYSGameWidget> MenuWidget;
	TSharedPtr<class SWidget> MenuContainerWidget;

};