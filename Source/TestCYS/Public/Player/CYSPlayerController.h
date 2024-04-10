// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CYSPlayerController.generated.h"

UCLASS()
class TESTCYS_API ACYSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void ShowDamage();
	void HiddenDamage();

};
