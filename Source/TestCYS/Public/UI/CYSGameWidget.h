// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"

class CYSGameWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(CYSGameWidget){}

	SLATE_ARGUMENT(TWeakObjectPtr<class ACYSHUDGame>, OwningHUD)

	SLATE_END_ARGS()

////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS

public:
	virtual void Construct(const FArguments& InArgs);
	virtual bool SupportsKeyboardFocus() const override { return true; };
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

////////////////////////////////////////////////////////////////////////////////////////////////
// VARIABLES

public:
	TWeakObjectPtr<class ACYSHUDGame> OwningHUD;
	TSharedPtr<SImage> Image;

	float CurrentValue = 0.2f;
	float MinValue = 0.2f;
	float MaxValue = 0.6f;
	float RateOfChange = 15.0f;
	bool bIncreasing = true;
};