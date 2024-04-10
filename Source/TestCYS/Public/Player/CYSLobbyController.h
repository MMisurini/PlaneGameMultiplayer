// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CYSLobbyController.generated.h"

UCLASS()
class TESTCYS_API ACYSLobbyController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ACYSLobbyController();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel);

	void UpdatePlayerList();
	void FinishUpdatePlayerList();

	UFUNCTION(Client, Reliable)
	void Client_UpdatePlayerList();

	void PlaneSelect(int32 Index);

	UFUNCTION(Server, Reliable)
	void Server_PlaneSelect(int32 Index);
	void Server_PlaneSelect_Implementation(int32 Index);

	void UpdatePlayerReady(int32 Count);

	UPROPERTY(Transient, Replicated)
	int32 PlayersReady;

	UPROPERTY(Transient, Replicated)
	int32 PlaneIndex;

};