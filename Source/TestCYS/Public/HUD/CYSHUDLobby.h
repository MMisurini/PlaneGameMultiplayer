// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CYSHUDLobby.generated.h"

UCLASS()
class TESTCYS_API ACYSHUDLobby : public AHUD
{
	GENERATED_BODY()
	
////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS

public:
	ACYSHUDLobby();

	virtual void BeginPlay() override;

	void UpdatePlayersList();
	void UpdatePlayersReady();
	void PlaneSelect(int32 Index);

////////////////////////////////////////////////////////////////////////////////////////////////
// VARIABLES

protected:
	TSharedPtr<class CYSLobbyWidget> MenuWidget;
	TSharedPtr<class SWidget> MenuContainerWidget;

	FSlateBrush PlaneBrush1;
	FSlateBrush PlaneBrush2;

};
