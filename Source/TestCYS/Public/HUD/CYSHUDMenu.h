// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CYSHUDMenu.generated.h"

UCLASS()
class TESTCYS_API ACYSHUDMenu : public AHUD
{
	GENERATED_BODY()
	
////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS

public:
	virtual void BeginPlay() override;

////////////////////////////////////////////////////////////////////////////////////////////////
// VARIABLES

protected:
	TSharedPtr<class CYSMainMenuWidget> MenuWidget;
	TSharedPtr<class SWidget> MenuContainerWidget;

};