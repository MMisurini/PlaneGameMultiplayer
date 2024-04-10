// Fill out your copyright notice in the Description page of Project Settings.


#include "Online/CYSGameMode.h"
#include "Player/CYSPlayer.h"
#include "Player/CYSPlayerController.h"
#include "HUD/CYSHUDGame.h"

//ENGINE
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

ACYSGameMode::ACYSGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	//DefaultPawnClass = ACYSPlayer::StaticClass();
	DefaultPawnClass = nullptr;
	PlayerControllerClass = ACYSPlayerController::StaticClass();
	HUDClass = ACYSHUDGame::StaticClass();

	bUseSeamlessTravel = true;
}

void ACYSGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

void ACYSGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);

	if (ACYSPlayerController* PC = Cast<ACYSPlayerController>(C))
	{
		ConnectedPlayers.Add(PC);
		
		FString Value = UGameplayStatics::ParseOption(OptionsString, FString::FromInt(C->PlayerState->GetPlayerId()));
		int32 Index = FCString::Atoi(*Value);
		
		FVector Location = FVector(0.f, 0.f, 10000.f);
		FRotator Rotation = FRotator(0.f, 0.f, 0.f);

		AActor* PlayerStart = FindPlayerStart(PC);
		if (PlayerStart)
		{
			Location = PlayerStart->GetActorLocation();
			Rotation = PlayerStart->GetActorRotation();
		}

		FActorSpawnParameters SpawnParameters;
		ACYSPlayer* Player = GetWorld()->SpawnActorDeferred<ACYSPlayer>(ACYSPlayer::StaticClass(), FTransform(Rotation, Location), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (Player)
		{
			Player->PlaneIndex = Index;
			UGameplayStatics::FinishSpawningActor(Player, FTransform(Rotation, Location));

			PC->Possess(Player);
		}
	}
}

void ACYSGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
}