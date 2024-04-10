// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CYSHUDLobby.h"
#include "UI/CYSLobbyWidget.h"
#include "Player/CYSLobbyController.h"

//ENGINE
#include "Engine/Engine.h"
#include "Widgets/SWeakWidget.h"

ACYSHUDLobby::ACYSHUDLobby()
{
	ConstructorHelpers::FObjectFinder<UTexture2D> PlaneImage1(TEXT("Texture2D'/Game/Slate/T_Plane1.T_Plane1'"));
	if (PlaneImage1.Object)
	{
		PlaneBrush1.SetResourceObject(PlaneImage1.Object);
		PlaneBrush1.SetImageSize(FVector2D(PlaneImage1.Object->GetSurfaceWidth(), PlaneImage1.Object->GetSurfaceHeight()));
	}

	ConstructorHelpers::FObjectFinder<UTexture2D> PlaneImage2(TEXT("Texture2D'/Game/Slate/T_Plane2.T_Plane2'"));
	if (PlaneImage2.Object)
	{
		PlaneBrush2.SetResourceObject(PlaneImage2.Object);
		PlaneBrush2.SetImageSize(FVector2D(PlaneImage2.Object->GetSurfaceWidth(), PlaneImage2.Object->GetSurfaceHeight()));
	}
}

void ACYSHUDLobby::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine && GEngine->GameViewport)
	{
		MenuWidget = SNew(CYSLobbyWidget).OwningHUD(this).PlaneTexture1(&PlaneBrush1).PlaneTexture2(&PlaneBrush2);
		GEngine->GameViewport->AddViewportWidgetContent(SAssignNew(MenuContainerWidget, SWeakWidget).PossiblyNullContent(MenuWidget.ToSharedRef()));

		if (PlayerOwner)
		{
			PlayerOwner->bShowMouseCursor = true;
			PlayerOwner->SetInputMode(FInputModeUIOnly());
		}
	}
}

void ACYSHUDLobby::UpdatePlayersList()
{
	if (MenuWidget.IsValid())
	{
		MenuWidget->UpdatePlayersList();
	}
}

void ACYSHUDLobby::UpdatePlayersReady()
{
	if (MenuWidget.IsValid())
	{
		MenuWidget->UpdatePlayersReady();
	}
}

void ACYSHUDLobby::PlaneSelect(int32 Index)
{
	if (ACYSLobbyController* PC = Cast<ACYSLobbyController>(PlayerOwner))
	{
		PC->PlaneSelect(Index);
	}
}