// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"

class CYSLobbyWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(CYSLobbyWidget){}

	SLATE_ARGUMENT(TWeakObjectPtr<class ACYSHUDLobby>, OwningHUD)
	SLATE_ARGUMENT(FSlateBrush*, PlaneTexture1)
	SLATE_ARGUMENT(FSlateBrush*, PlaneTexture2)

	SLATE_END_ARGS()

////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS

public:
	virtual void Construct(const FArguments& InArgs);
	virtual bool SupportsKeyboardFocus() const override { return true; };
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void UpdatePlayersList();
	void UpdatePlayersReady();

	FReply SelectPlane1() const;
	FReply SelectPlane2() const;

////////////////////////////////////////////////////////////////////////////////////////////////
// VARIABLES

public:
	TWeakObjectPtr<class ACYSHUDLobby> OwningHUD;
	FSlateBrush* PlaneTexture1;
	FSlateBrush* PlaneTexture2;
	TSharedPtr<SVerticalBox> PlayerList;
	TSharedPtr<SOverlay> Messages;
	TSharedPtr<STextBlock> Title;

	TSharedPtr<SButton> Plane1;
	TSharedPtr<SButton> Plane2;

	bool bIsLoading = false;
};
