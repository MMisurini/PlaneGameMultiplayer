// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Interfaces/OnlineSessionInterface.h"

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CYSSessionSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCustomCreateSessionDelegate, bool Successful);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCustomStartSessionComplete, bool Successful);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCustomEndSessionComplete, bool Successful);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCustomDestroySessionComplete, bool Successful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCustomFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool Successful);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCustomJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);

UCLASS()
class TESTCYS_API UCYSSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS

public:
	UCYSSessionSubsystem();

	void CreateSession(int32 NumPublicConnections, bool IsLANMatch);
	void StartSession();
	void EndSession();
	void DestroySession();
	void FindSessions(int32 MaxSearchResults, bool IsLANQuery);
	void JoinGameSession(const FOnlineSessionSearchResult& SessionResult);

protected:
	void OnCreateSessionCompleted(FName SessionName, bool Successful);
	void OnStartSessionCompleted(FName SessionName, bool Successful);
	void OnEndSessionCompleted(FName SessionName, bool Successful);
	void OnDestroySessionCompleted(FName SessionName, bool Successful);
	void OnFindSessionsCompleted(bool Successful);
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

public:
	bool TryTravelToCurrentSession();

////////////////////////////////////////////////////////////////////////////////////////////////
// VARIABLES

public:
	FOnCustomCreateSessionDelegate OnCreateSessionCompleteEvent;
	FOnCustomStartSessionComplete OnStartSessionCompleteEvent;
	FOnCustomEndSessionComplete OnEndSessionCompleteEvent;
	FOnCustomDestroySessionComplete OnDestroySessionCompleteEvent;
	FOnCustomFindSessionsComplete OnFindSessionsCompleteEvent;
	FOnCustomJoinSessionComplete OnJoinGameSessionCompleteEvent;

private:
	//Create Session
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	
	//Start Session
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	//End Session
	FOnEndSessionCompleteDelegate EndSessionCompleteDelegate;
	FDelegateHandle EndSessionCompleteDelegateHandle;

	//Destroy Session
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	//Find Session
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	//Join Session
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
};
