// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CYSLobbyController.h"
#include "HUD/CYSHUDLobby.h"

//ENGINE
#include "Net/UnrealNetwork.h"

ACYSLobbyController::ACYSLobbyController()
{
}

void ACYSLobbyController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACYSLobbyController, PlaneIndex);
    DOREPLIFETIME(ACYSLobbyController, PlayersReady);
}

void ACYSLobbyController::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
    Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);

    ACYSHUDLobby* LobbyHUD = Cast<ACYSHUDLobby>(GetHUD());
    if (LobbyHUD)
    {
        LobbyHUD->UpdatePlayersReady();
    }
}

void ACYSLobbyController::UpdatePlayerList()
{
    if (GetLocalRole() == ROLE_Authority)
    {
        if (IsLocalController())
        {
            FinishUpdatePlayerList();
        }
        else
        {
            Client_UpdatePlayerList();
        }
    }

    FinishUpdatePlayerList();
}

void ACYSLobbyController::FinishUpdatePlayerList()
{
    ACYSHUDLobby* LobbyHUD = Cast<ACYSHUDLobby>(GetHUD());
    if (LobbyHUD)
    {
        LobbyHUD->UpdatePlayersList();
    }
}

void ACYSLobbyController::Client_UpdatePlayerList_Implementation()
{
	UpdatePlayerList();
}

void ACYSLobbyController::PlaneSelect(int32 Index)
{
    if (GetLocalRole() < ROLE_Authority)
    {
        Server_PlaneSelect(Index);
    }

    PlaneIndex = Index;
}

void ACYSLobbyController::Server_PlaneSelect_Implementation(int32 Index)
{
    PlaneSelect(Index);
}

void ACYSLobbyController::UpdatePlayerReady(int32 Count)
{
    PlayersReady = Count;
}