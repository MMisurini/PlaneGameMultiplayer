// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"

class CYSMainMenuWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(CYSMainMenuWidget){}
	SLATE_ARGUMENT(TWeakObjectPtr<class ACYSHUDMenu>, OwningHUD)
	SLATE_END_ARGS()

////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS

public:
	virtual void Construct(const FArguments& InArgs);
	virtual bool SupportsKeyboardFocus() const override { return true; };

	FReply OnBackClicked() const;
	FReply OnCreateRoomClicked() const;
	FReply OnConnectRoomClicked() const;
	FReply OnQuitClicked() const;

private:
	void UpdateRoomsList(const TArray<FOnlineSessionSearchResult>& NewList) const;

////////////////////////////////////////////////////////////////////////////////////////////////
// VARIABLES

public:
	TWeakObjectPtr<class ACYSHUDMenu> OwningHUD;
	TSharedPtr<SWidgetSwitcher> Switcher;
	TSharedPtr<SWidgetSwitcher> RoomSwitcher;
	TSharedPtr<SVerticalBox> RoomsList;
};
