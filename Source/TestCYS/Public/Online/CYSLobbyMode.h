// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CYSLobbyMode.generated.h"

UCLASS()
class TESTCYS_API ACYSLobbyMode : public AGameModeBase
{
	GENERATED_BODY()
	
////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS

public:
	ACYSLobbyMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void BeginPlay() override;

private:
	void UpdateEssentials();
	void UpdatePlayersList();
	void UpdatePlayersPlaneSelected();

////////////////////////////////////////////////////////////////////////////////////////////////
// VARIABLES

protected:
	UPROPERTY()
	FTimerHandle TimerHandle_Loop;

	UPROPERTY()
	TArray<APlayerController*> ConnectedPlayers;

};