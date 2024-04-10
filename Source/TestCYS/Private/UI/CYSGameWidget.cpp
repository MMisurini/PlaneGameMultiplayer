// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CYSGameWidget.h"
#include "HUD/CYSHUDGame.h"

//ENGINE
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "Game"

void CYSGameWidget::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	OwningHUD = InArgs._OwningHUD;

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(Image, SImage)
			.ColorAndOpacity(FColor(255, 0, 0, 0))
		]
	];
}

void CYSGameWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

    if (bIncreasing)
    {
        CurrentValue += RateOfChange * UGameplayStatics::GetWorldDeltaSeconds(OwningHUD->GetWorld());
        Image->SetColorAndOpacity(FLinearColor(255, 0, 0, CurrentValue));
    }
    else
    {
        CurrentValue -= RateOfChange * UGameplayStatics::GetWorldDeltaSeconds(OwningHUD->GetWorld());
        Image->SetColorAndOpacity(FLinearColor(255, 0, 0, CurrentValue));
    }

    if (CurrentValue >= MaxValue)
    {
        bIncreasing = false;
    }
    else if (CurrentValue <= MinValue)
    {
        bIncreasing = true;
    }
}

#undef LOCTEXT_NAMESPACE