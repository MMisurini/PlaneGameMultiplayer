// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CYSPlayer.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UNiagaraComponent;

USTRUCT(BlueprintType)
struct FInputState
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	int Tick;

	UPROPERTY()
	float AxisForward;

	UPROPERTY()
	float AxisRight;

	UPROPERTY()
	float AxisTurn;

	UPROPERTY()
	float AxisLookUp;

	FInputState()
		: Tick(0)
		, AxisForward(0)
		, AxisRight(0)
		, AxisTurn(0)
		, AxisLookUp(0)
	{
	}

	void Reset()
	{
		AxisForward = 0.f;
		AxisRight = 0.f;
		AxisTurn = 0.f;
		AxisLookUp = 0.f;
	}

	bool IsSame(FInputState s)
	{
		return	AxisForward == s.AxisForward &&
			AxisRight == s.AxisRight &&
			AxisTurn == s.AxisTurn &&
			AxisLookUp == s.AxisLookUp;
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		//Tick
		Ar << Tick;

		//Forward
		uint8 B = (AxisForward != 0);
		Ar.SerializeBits(&B, 1);
		if (B) Ar << AxisForward; else AxisForward = 0;

		//Right
		B = (AxisRight != 0);
		Ar.SerializeBits(&B, 1);
		if (B) Ar << AxisRight; else AxisRight = 0;

		//Turn
		B = (AxisTurn != 0);
		Ar.SerializeBits(&B, 1);
		if (B) Ar << AxisTurn; else AxisTurn = 0;

		//Look Up
		B = (AxisLookUp != 0);
		Ar.SerializeBits(&B, 1);
		if (B) Ar << AxisLookUp; else AxisLookUp = 0;

		bOutSuccess = true;
		return true;
	}
};

template<> struct TStructOpsTypeTraits<FInputState> : public TStructOpsTypeTraitsBase2<FInputState>
{
	enum { WithNetSerializer = true };
};

USTRUCT()
struct FInputStateBuffer
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	TArray<FInputState> mBuffer;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		//Array size
		uint8 count = mBuffer.Num();
		Ar << count;
		mBuffer.SetNum(count);

		//Reading
		if (Ar.IsLoading())
		{
			for (int i = 0; i < count; i++)
			{
				if (i == 0)
				{
					mBuffer[i].NetSerialize(Ar, Map, bOutSuccess);
				}
				else
				{
					bool duplicate = false;
					Ar.SerializeBits(&duplicate, 1);
					if (duplicate)
					{
						uint8 sameCount = 0;
						Ar << sameCount;
						for (int j = 0; j < sameCount; j++)
						{
							mBuffer[i + j] = mBuffer[i + j - 1];
							mBuffer[i + j].Tick++;
						}
						i += (sameCount - 1);
					}
					else
					{
						mBuffer[i].NetSerialize(Ar, Map, bOutSuccess);
					}
				}
			}
		}
		//writing
		else
		{
			uint8 sameCount = 0;
			FInputState last = mBuffer[0];
			for (int i = 0; i < count; i++)
			{
				FInputState state = mBuffer[i];
				bool same = i > 0 && state.IsSame(last);
				if (same) sameCount++;
				else
				{
					//check same count
					bool B = sameCount > 0;
					if (B)
					{
						Ar.SerializeBits(&B, 1);
						Ar << sameCount;
						sameCount = 0;
					}
					if (i > 0)
					{
						B = false;
						Ar.SerializeBits(&B, 1);
					}
					mBuffer[i].NetSerialize(Ar, Map, bOutSuccess);
				}
				last = state;
			}
			//check same count
			bool B = sameCount > 0;
			if (B)
			{
				Ar.SerializeBits(&B, 1);
				Ar << sameCount;
			}
		}

		bOutSuccess = true;
		return true;
	}
};

template<> struct TStructOpsTypeTraits<FInputStateBuffer> : public TStructOpsTypeTraitsBase2<FInputStateBuffer>
{
	enum { WithNetSerializer = true };
};

UCLASS()
class TESTCYS_API ACYSPlayer : public APawn
{
	GENERATED_BODY()

////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS

public:
	ACYSPlayer();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

private:
	void BindInputMoveForward(float Value);
	void BindInputMoveRight(float Value);
	void BindInputTurn(float Value);
	void BindInputLookUP(float Value);

	void BindInputFirePressed();
	void BindInputFireOn();
	void BindInputFireReleased();

	void UpdateMesh(int32 Index);
	void Fire();

	UFUNCTION(Unreliable, Server, WithValidation)
	void Server_Fire();
	bool Server_Fire_Validate() { return true; }

	void CustomTakeDamage();
	void CheckTakeDamage();

	UFUNCTION(Reliable, Client)
	void Client_CustomTakeDamage();

	//~~ New Movement

	void ForwardMovement(float DeltaTime);
	void RotationMovement(float DeltaTime);
	void SetClientTransform();

	UFUNCTION(Unreliable, Server)
	void Server_UpdateData(float CustomYaw, float CustomPitch, float CustomRoll, FVector CustomLocation, FRotator CustomRotation);

////////////////////////////////////////////////////////////////////////////////////////////////
// VARIABLES

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CollisionMesh;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* PlaneMesh;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;

public:
	UPROPERTY(Replicated)
	int32 PlaneIndex = 0;

private:
	UPROPERTY(Transient)
	bool TakingDamage;

	UPROPERTY(Transient)
	FTimerHandle TimerHandle_Fire;

	UPROPERTY(Transient)
	FTimerHandle TimerHandle_TakeDamage;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//~~ New Movement

	UPROPERTY(Transient, Replicated)
	float YawRep;

	UPROPERTY(Transient, Replicated)
	float PitchRep;

	UPROPERTY(Transient, Replicated)
	float RollRep;

	UPROPERTY(Transient, Replicated)
	FVector ActorLocation;

	UPROPERTY(Transient, Replicated)
	FRotator ActorRotation;

	//~~ Variables Configs

	UPROPERTY()
	float TimeSinceUpdate = 0.f;

	UPROPERTY()
	float Realtime = 0.f;

	UPROPERTY()
	float CurrentSpeed = 3000.f;

	UPROPERTY()
	float TargetSpeed = 3000.f;

	UPROPERTY()
	float Acceleration = 0.f;

	UPROPERTY()
	float MaxSpeed = 15000.f;

	UPROPERTY()
	float MinSpeed = 0.f;

	UPROPERTY()
	float SpeedInterp = 5000.f;

	//~~ Roll

	UPROPERTY()
	float Roll = 0.f;

	UPROPERTY()
	float RollMax = 130.f;

	UPROPERTY()
	float InterpRoll = 800.f;

	UPROPERTY()
	float DeltaRoll = 0.f;

	//~~ Pitch

	UPROPERTY()
	float Pitch = 0.f;

	UPROPERTY()
	float PitchMax = 50.f;

	UPROPERTY()
	float InterpPitch = 200.f;

	UPROPERTY()
	float DeltaPitch = 0.f;

	//~~ Yaw

	UPROPERTY()
	float Yaw = 0.f;

	UPROPERTY()
	float YawMax = 50.f;

	UPROPERTY()
	float InterpYaw = 200.f;

	UPROPERTY()
	float DeltaYaw = 0.f;

};