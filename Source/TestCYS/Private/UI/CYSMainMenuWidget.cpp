// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CYSMainMenuWidget.h"
#include "UI/CYSSessionWidget.h"
#include "HUD/CYSHUDMenu.h"
#include "Session/CYSSessionSubsystem.h"

//ENGINE
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "MainMenu"

void CYSMainMenuWidget::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	OwningHUD = InArgs._OwningHUD;

	const FMargin ContentPadding = FMargin(500.f, 300.f);
	const FMargin ButtonPadding = FMargin(10.f);

	const FText TitleText = LOCTEXT("GameTitle", "CYS Plane");
	const FText CreateText = LOCTEXT("CreateRoom", "Create Room");
	const FText ConnectText = LOCTEXT("ConnectRoom", "Rooms");
	const FText QuitText = LOCTEXT("QuitGame", "Quit");
	const FText BackText = LOCTEXT("Back", "Voltar");

	FSlateFontInfo ButtonTextStyle = FCoreStyle::Get().GetFontStyle("EmbossedText");
	ButtonTextStyle.Size = 40.f;

	FSlateFontInfo BackTextStyle = ButtonTextStyle;
	BackTextStyle.Size = 20.f;

	FSlateFontInfo TitleTextStyle = ButtonTextStyle;
	TitleTextStyle.Size = 60.f;

	ChildSlot
	[
		SAssignNew(Switcher, SWidgetSwitcher)

		//Main Menu
		+ SWidgetSwitcher::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
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
			.VAlign(VAlign_Top)
			.Padding(ContentPadding)
			[
				SNew(SVerticalBox)

				//Title Text
				+ SVerticalBox::Slot()
				.Padding(0.f, 0.f, 0.f, 50.f)
				[
					SNew(STextBlock)
					.Text(TitleText)
					.Font(TitleTextStyle)
					.Justification(ETextJustify::Center)
				]

				//Create Room Text
				+ SVerticalBox::Slot()
				.Padding(ButtonPadding)
				[
					SNew(SButton)
					.OnClicked(this, &CYSMainMenuWidget::OnCreateRoomClicked)
					[
						SNew(STextBlock)
						.Text(CreateText)
						.Font(ButtonTextStyle)
						.Justification(ETextJustify::Center)
					]
				]

				//Connect Room Text
				+ SVerticalBox::Slot()
				.Padding(ButtonPadding)
				[
					SNew(SButton)
					.OnClicked(this, &CYSMainMenuWidget::OnConnectRoomClicked)
					[
						SNew(STextBlock)
						.Text(ConnectText)
						.Font(ButtonTextStyle)
						.Justification(ETextJustify::Center)
					]
				]

				//Quit Text
				+ SVerticalBox::Slot()
				.Padding(ButtonPadding)
				[
					SNew(SButton)
					.OnClicked(this, &CYSMainMenuWidget::OnQuitClicked)
					[
						SNew(STextBlock)
						.Text(QuitText)
						.Font(ButtonTextStyle)
						.Justification(ETextJustify::Center)
					]
				]
			]
		]

		//Find Sessions
		+ SWidgetSwitcher::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
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

				//Title Text
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(STextBlock)
					.Text(TitleText)
					.Font(TitleTextStyle)
					.Justification(ETextJustify::Center)
				]

				//Rooms List
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SAssignNew(RoomSwitcher, SWidgetSwitcher)

					//Loading
					+ SWidgetSwitcher::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SCircularThrobber)
						.Radius(30)
					]

					//List
					+ SWidgetSwitcher::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SAssignNew(RoomsList, SVerticalBox)
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
					]
				]
			]

			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			.Padding(FMargin(0.f, 310.f, 510.f, 0.f))
			[
				SNew(SButton)
				.OnClicked(this, &CYSMainMenuWidget::OnBackClicked)
				[
					SNew(STextBlock)
					.Text(BackText)
					.Font(BackTextStyle)
					.Justification(ETextJustify::Center)
				]
			]
		]
	];
}

FReply CYSMainMenuWidget::OnBackClicked() const
{
	Switcher->SetActiveWidgetIndex(0);
	RoomSwitcher->SetActiveWidgetIndex(0);

	return FReply::Handled();
}

FReply CYSMainMenuWidget::OnCreateRoomClicked() const
{
	if (OwningHUD.IsValid())
	{
		UCYSSessionSubsystem* Subsystem = OwningHUD->GetWorld()->GetGameInstance()->GetSubsystem<UCYSSessionSubsystem>();
		if (Subsystem) 
		{
			Subsystem->OnCreateSessionCompleteEvent.AddLambda([this, Subsystem](bool bWasSuccessful) 
			{
				if (bWasSuccessful)
				{
					UGameplayStatics::OpenLevel(OwningHUD->GetWorld(), FName("Map_Lobby"), true, TEXT("?listen"));
					Subsystem->StartSession();
				}
			});

			Subsystem->OnStartSessionCompleteEvent.AddLambda([this, Subsystem](bool bWasSuccessful)
			{
				
			});

			Subsystem->CreateSession(2, false);
		}
	}

	return FReply::Handled();
}

FReply CYSMainMenuWidget::OnConnectRoomClicked() const
{
	if (Switcher.IsValid())
	{
		Switcher->SetActiveWidgetIndex(1);
	}

	if (RoomSwitcher.IsValid())
	{
		RoomSwitcher->SetActiveWidgetIndex(0);
	}
	
	if (OwningHUD.IsValid())
	{
		UCYSSessionSubsystem* Subsystem = OwningHUD->GetWorld()->GetGameInstance()->GetSubsystem<UCYSSessionSubsystem>();
		if (Subsystem)
		{
			Subsystem->OnFindSessionsCompleteEvent.AddLambda([this](const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
			{
				if (bWasSuccessful)
				{
					if (RoomSwitcher.IsValid())
					{
						RoomSwitcher->SetActiveWidgetIndex(1);
					}
					
					UpdateRoomsList(SessionResults);
				}
			});

			Subsystem->FindSessions(10, false);
		}
	}

	return FReply::Handled();
}

FReply CYSMainMenuWidget::OnQuitClicked() const
{
	if (OwningHUD.IsValid())
	{
		if (APlayerController* PC = OwningHUD->PlayerOwner)
		{
			PC->ConsoleCommand("quit");
		}
	}

	return FReply::Handled();
}

void CYSMainMenuWidget::UpdateRoomsList(const TArray<FOnlineSessionSearchResult>& NewList) const
{
	RoomsList->ClearChildren();

	for (auto& session : NewList)
	{
		RoomsList->AddSlot()
		.AutoHeight()
		[
			SNew(CYSSessionWidget)
			.OwningHUD(OwningHUD)
			.Session(session)
		];
	}
}

#undef LOCTEXT_NAMESPACE