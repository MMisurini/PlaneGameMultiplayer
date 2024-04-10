// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"
#include "OnlineSessionSettings.h"

class CYSSessionWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(CYSSessionWidget){}
		SLATE_ARGUMENT(TWeakObjectPtr<class ACYSHUDMenu>, OwningHUD)
		SLATE_ATTRIBUTE(FOnlineSessionSearchResult, Session)
	SLATE_END_ARGS()

////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS

public:
	virtual void Construct(const FArguments& InArgs);
	virtual bool SupportsKeyboardFocus() const override { return true; };
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	FReply OnSessionSelected() const;

////////////////////////////////////////////////////////////////////////////////////////////////
// VARIABLES

public:
	TWeakObjectPtr<class ACYSHUDMenu> OwningHUD;

	UPROPERTY(Transient)
	FOnlineSessionSearchResult SessionResult;

};