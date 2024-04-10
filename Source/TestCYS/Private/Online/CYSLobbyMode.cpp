// Fill out your copyright notice in the Description page of Project Settings.


#include "Online/CYSLobbyMode.h"
#include "Player/CYSLobbyController.h"
#include "HUD/CYSHUDLobby.h"
#include "Global/CYSGameInstance.h"

//ENGINE
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerState.h"

ACYSLobbyMode::ACYSLobbyMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = ACYSLobbyController::StaticClass();
	HUDClass = ACYSHUDLobby::StaticClass();

	bUseSeamlessTravel = true;
}

void ACYSLobbyMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ConnectedPlayers.Add(NewPlayer);
	GetWorldTimerManager().SetTimerForNextTick(this, &ACYSLobbyMode::UpdateEssentials);
}

void ACYSLobbyMode::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(TimerHandle_Loop, this, &ACYSLobbyMode::UpdateEssentials, 0.25f, true);
}

void ACYSLobbyMode::UpdateEssentials()
{
	UpdatePlayersList();
	UpdatePlayersPlaneSelected();
}

void ACYSLobbyMode::UpdatePlayersList()
{
	for (auto& PC : ConnectedPlayers)
	{
		if (ACYSLobbyController* NPC = Cast<ACYSLobbyController>(PC))
		{
			NPC->UpdatePlayerList();
		}	
	}
}

void ACYSLobbyMode::UpdatePlayersPlaneSelected()
{
	int32 PlayersReady = 0;
	for (auto& PC : ConnectedPlayers)
	{
		if (ACYSLobbyController* NPC = Cast<ACYSLobbyController>(PC))
		{
			if (NPC->PlaneIndex != 0)
			{
				PlayersReady++;
			}
		}
	}

	for (auto& PC : ConnectedPlayers)
	{
		if (ACYSLobbyController* NPC = Cast<ACYSLobbyController>(PC))
		{
			NPC->UpdatePlayerReady(PlayersReady);
		}
	}

	if (PlayersReady >= ConnectedPlayers.Num())
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_Loop);

		FString Opt;
		for (auto& PC : ConnectedPlayers)
		{
			if (ACYSLobbyController* NPC = Cast<ACYSLobbyController>(PC))
			{
				Opt += FString::Printf(TEXT("?%s=%d"), *FString::FromInt(NPC->PlayerState->GetPlayerId()), NPC->PlaneIndex);
			}
		}

		FString Cmd = TEXT("ServerTravel");
		FString MapPath = TEXT("/Game/Maps/Map_Game");

		FString Command = FString::Printf(TEXT("%s %s%s"), *Cmd, *MapPath, *Opt);
		if (IConsoleManager::Get().ProcessUserConsoleInput(*Command, *GLog, GetWorld()) == false)
		{
			if (GEngine)
			{
				GEngine->Exec(GetWorld(), *Command);
			}
		}
	}
}