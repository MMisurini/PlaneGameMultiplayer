// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CYSLobbyWidget.h"
#include "HUD/CYSHUDLobby.h"
#include "Player/CYSLobbyController.h"

//ENGINE
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetStringLibrary.h"

#define LOCTEXT_NAMESPACE "Lobby"

void CYSLobbyWidget::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	OwningHUD = InArgs._OwningHUD;
	PlaneTexture1 = InArgs._PlaneTexture1;
	PlaneTexture2 = InArgs._PlaneTexture2;

	const FMargin ContentPadding = FMargin(300.f, 300.f);

	const FText TitleText = LOCTEXT("Title", "Select a plane.");
	const FText PlayersText = FText::FromString("0/0");
	const FText Plane1Text = LOCTEXT("Plane1", "Plane 1");
	const FText Plane2Text = LOCTEXT("Plane2", "Plane 2");

	FSlateFontInfo ButtonTextStyle = FCoreStyle::Get().GetFontStyle("EmbossedText");
	ButtonTextStyle.Size = 40.f;

	FSlateFontInfo TitleTextStyle = ButtonTextStyle;
	TitleTextStyle.Size = 30.f;

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SImage)
			.ColorAndOpacity(FColor::Black)
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(ContentPadding)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(10.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(TitleText)
					.Font(TitleTextStyle)
				]

				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Fill)
				[
					SAssignNew(Title, STextBlock)
					.Text(PlayersText)
					.Font(TitleTextStyle)
				]
			]

			//Content
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SHorizontalBox)

				//Player List
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(300)
					[
						SNew(SBorder)
						[
							SAssignNew(PlayerList, SVerticalBox)
							+ SVerticalBox::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
						]
					]
				]

				// Planes
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SHorizontalBox)

					//Plane 1
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.Padding(5.f, 0.f, 5.f, 0.f)
					[
						SAssignNew(Plane1, SButton)
						.Cursor(EMouseCursor::Type::Hand)
						.OnClicked(this, &CYSLobbyWidget::SelectPlane1)
						[
							SNew(SImage)
							.Image(PlaneTexture1)
						]
					]

					//Plane 2
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.Padding(5.f, 0.f, 0.f, 0.f)
					[
						SAssignNew(Plane2, SButton)
						.Cursor(EMouseCursor::Type::Hand)
						.OnClicked(this, &CYSLobbyWidget::SelectPlane2)
						[
							SNew(SImage)
							.Image(PlaneTexture2)
						]
					]
				]
			]
		]
	];
}

void CYSLobbyWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (OwningHUD.IsValid() && !bIsLoading)
	{
		if (ACYSLobbyController* NPC = Cast<ACYSLobbyController>(OwningHUD->GetOwningPlayerController()))
		{
			int32 Current = NPC->PlayersReady;
			int32 Max = OwningHUD->GetWorld()->GetGameState()->PlayerArray.Num();

			Title->SetText(FText::AsCultureInvariant(FString::Printf(TEXT("%d/%d"), Current, Max)));
		}
	}

}

void CYSLobbyWidget::UpdatePlayersList()
{
	PlayerList->ClearChildren();

	FSlateFontInfo ButtonTextStyle = FCoreStyle::Get().GetFontStyle("EmbossedText");
	ButtonTextStyle.Size = 16.f;

	if (OwningHUD.IsValid())
	{
		if (OwningHUD->GetWorld())
		{
			for (auto state : OwningHUD->GetWorld()->GetGameState()->PlayerArray)
			{
				if (state)
				{
					PlayerList->AddSlot()
					.AutoHeight()
					[
						SNew(SButton)
						.ContentPadding(FMargin(10.f))
						[
							SNew(STextBlock)
							.Text(FText::FromString(state->GetPlayerName()))
							.Font(ButtonTextStyle)
							.ColorAndOpacity(FColor::Black)
							.Justification(ETextJustify::Center)
						]
					];
				}
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(0, 0.2f, FColor::Red, FString::Printf(TEXT("World Is Not Valid")));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(0, 0.2f, FColor::Red, FString::Printf(TEXT("OwningHUD Is Not Valid")));
	}
}

void CYSLobbyWidget::UpdatePlayersReady()
{
	bIsLoading = true;
	Title->SetText(FText::AsCultureInvariant("LOADING..."));
}

FReply CYSLobbyWidget::SelectPlane1() const
{
	if (OwningHUD.IsValid())
	{
		OwningHUD->PlaneSelect(1);
		Plane1->SetColorAndOpacity(FColor::Red);
		Plane1->SetVisibility(EVisibility::HitTestInvisible);
		Plane2->SetVisibility(EVisibility::HitTestInvisible);
	}

	return FReply::Handled();
}

FReply CYSLobbyWidget::SelectPlane2() const
{
	if (OwningHUD.IsValid())
	{
		OwningHUD->PlaneSelect(2);
		Plane2->SetColorAndOpacity(FColor::Red);
		Plane1->SetVisibility(EVisibility::HitTestInvisible);
		Plane2->SetVisibility(EVisibility::HitTestInvisible);
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE