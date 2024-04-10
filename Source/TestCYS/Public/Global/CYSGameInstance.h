// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CYSGameInstance.generated.h"

UCLASS()
class TESTCYS_API UCYSGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS

public:

////////////////////////////////////////////////////////////////////////////////////////////////
// VARIABLES

	//~~ SERVER

public:
	TMap<FUniqueNetIdRepl, int32> PlayersSettings;

};
