// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CYSGameMode.generated.h"

class ACYSPlayerController;

UCLASS()
class TESTCYS_API ACYSGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS

public:
	ACYSGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	virtual void PostSeamlessTravel() override;

////////////////////////////////////////////////////////////////////////////////////////////////
// VARIABLES

protected:
	UPROPERTY(Transient)
	TArray<APlayerController*> ConnectedPlayers;

};
