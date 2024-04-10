// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CYSSessionWidget.h"
#include "HUD/CYSHUDMenu.h"
#include "Session/CYSSessionSubsystem.h"

#define LOCTEXT_NAMESPACE "Session"

void CYSSessionWidget::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	OwningHUD = InArgs._OwningHUD;
	SessionResult = InArgs._Session.Get();

	FSlateFontInfo ButtonTextStyle = FCoreStyle::Get().GetFontStyle("EmbossedText");
	ButtonTextStyle.Size = 20.f;

	ChildSlot
	[
		SNew(SBox)
		[
			SNew(SButton)
			.ContentPadding(10.f)
			.OnClicked(this, &CYSSessionWidget::OnSessionSelected)
			[
				SNew(STextBlock)
				.Text(FText::AsCultureInvariant(SessionResult.Session.OwningUserName))
				.Font(ButtonTextStyle)
			]
		]
	];
}

void CYSSessionWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

}

FReply CYSSessionWidget::OnSessionSelected() const
{
	if (OwningHUD.IsValid())
	{
		UCYSSessionSubsystem* Subsystem = OwningHUD->GetWorld()->GetGameInstance()->GetSubsystem<UCYSSessionSubsystem>();
		if (Subsystem)
		{
			Subsystem->OnJoinGameSessionCompleteEvent.AddLambda([this, Subsystem](EOnJoinSessionCompleteResult::Type Result)
			{
				if (Result == EOnJoinSessionCompleteResult::Type::Success)
				{
					Subsystem->TryTravelToCurrentSession();
				}
			});

			Subsystem->JoinGameSession(SessionResult);
		}
	}
	
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE