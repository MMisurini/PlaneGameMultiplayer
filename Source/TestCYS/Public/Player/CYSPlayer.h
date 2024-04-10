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

	void Throttle();
	void Speed();

	void SetInputs(float Forward, float Right, float Turn, float LookUp);

	void InputForward(float Value);
	void InputRight(float Value);
	void InputTurn(float Value);
	void InputLookUp(float Value);

	//~~ Replications
	void UpdateReplication(float DeltaTime);
	void UpdateInputs();
	void SendInputs(float DeltaTime);

	void ReceiveInput(const FInputState& InputState);

	UFUNCTION(Unreliable, Server, WithValidation)
	void Server_ReceiveInputs(const FInputStateBuffer& Inputs);
	bool Server_ReceiveInputs_Validate(const FInputStateBuffer& Inputs) { return true; }

	void UpdateMesh(int32 Index);
	void Fire();

	UFUNCTION(Unreliable, Server, WithValidation)
	void Server_Fire();
	bool Server_Fire_Validate() { return true; }

	void CustomTakeDamage();
	void CheckTakeDamage();

	UFUNCTION(Reliable, Client)
	void Client_CustomTakeDamage();

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

	UPROPERTY(Transient, Replicated)
	bool ThrottleUp;
	
	UPROPERTY(Transient, Replicated)
	bool ThrottleDown;

	bool ZeroThrottle;

	float CurrentThrottle = 5;
	float Rudder;

	float AxisForward;
	float AxisRight;
	float AxisTurn;
	float AxisLookUp;

	//~~ Replications

	int cysTick;

	//Config
	uint8 mInputSendRate = 60;
	uint8 cysInputBufferSize = 8;
	uint8 cysDoubleConsume;

	//Time
	UPROPERTY(Replicated)
	float cysServerTime;

	float cysLocalTime;
	float cysInputSendTime;

	//Buffer
	UPROPERTY(Replicated)
	FInputState cysLastInput;

	FInputStateBuffer cysInputBuffer;
};