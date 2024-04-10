// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CYSProjectile.generated.h"

class UNiagaraComponent;

UCLASS()
class TESTCYS_API ACYSProjectile : public AActor
{
	GENERATED_BODY()
	
////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS

public:	
	ACYSProjectile();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	void Speed();

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

////////////////////////////////////////////////////////////////////////////////////////////////
// VARIABLES

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Collision;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* Muzzle;

};
