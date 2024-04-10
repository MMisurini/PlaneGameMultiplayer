// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CYSPlayer.h"
#include "Player/CYSPlayerController.h"
#include "Projectile/CYSProjectile.h"

//ENGINE
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Engine/AssetManager.h"

// Sets default values
ACYSPlayer::ACYSPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

    CollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Collision Mesh"));
    CollisionMesh->SetGenerateOverlapEvents(true);
    CollisionMesh->SetSimulatePhysics(true);
    CollisionMesh->SetEnableGravity(false);
    CollisionMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
    CollisionMesh->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
    CollisionMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    CollisionMesh->SetHiddenInGame(true);
    RootComponent = CollisionMesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CuboMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
    if (CuboMeshAsset.Succeeded())
    {
        CollisionMesh->SetStaticMesh(CuboMeshAsset.Object);
        CollisionMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    }

    PlaneMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Plane Mesh"));
    PlaneMesh->SetGenerateOverlapEvents(true);
    PlaneMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
    PlaneMesh->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
    PlaneMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    PlaneMesh->SetupAttachment(CollisionMesh);

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
    SpringArm->TargetArmLength = 2000.f;
    SpringArm->SetRelativeLocation(FVector(0.f, 0.f, 280.f));
    SpringArm->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));
    SpringArm->bEnableCameraRotationLag = true;
    SpringArm->CameraRotationLagSpeed = 10.f;
    SpringArm->SetupAttachment(PlaneMesh);

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetRelativeRotation(FRotator(10.f, 0.f, 0.f));
    Camera->SetupAttachment(SpringArm);

    bReplicates = true;
    SetReplicateMovement(true);
}

const TCHAR* PlaneMeshPath_1 = TEXT("SkeletalMesh'/Game/Assets/VigilanteContent/Vehicles/East_Fighter_Su33/SK_East_Fighter_Su33.SK_East_Fighter_Su33'");
const TCHAR* PlaneMeshPath_2 = TEXT("SkeletalMesh'/Game/Assets/VigilanteContent/Vehicles/West_Fighter_F15E/SK_West_Fighter_F15E.SK_West_Fighter_F15E'");

void ACYSPlayer::BeginPlay()
{
	Super::BeginPlay();
	
    if (IsLocallyControlled())
    {
        if (ACYSPlayerController* PC = Cast<ACYSPlayerController>(GetController()))
        {
            PC->bShowMouseCursor = false;
            PC->SetInputMode(FInputModeGameOnly());
        }

        UWidgetLayoutLibrary::RemoveAllWidgets(GetWorld());
    }

    UpdateMesh(PlaneIndex);
}

// Called every frame
void ACYSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    UpdateReplication(DeltaTime);

    if (GetLocalRole() == ROLE_Authority)
    {
        Throttle();

        FVector Torque = UKismetMathLibrary::NegateVector(CollisionMesh->GetPhysicsAngularVelocityInDegrees()) / 0.75f;
        CollisionMesh->AddTorqueInDegrees(Torque, NAME_None, true);
        CollisionMesh->AddForce(FVector(0, 0, -1000.f), NAME_None, true);

        Speed();

        if (ZeroThrottle)
        {
            CurrentThrottle = UKismetMathLibrary::FInterpTo_Constant(CurrentThrottle, 0.f, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), 500.f);
        }
    }  
}

// Called to bind functionality to input
void ACYSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACYSPlayer::BindInputFirePressed);
    PlayerInputComponent->BindAction("Fire", IE_Released, this, &ACYSPlayer::BindInputFireReleased);

    PlayerInputComponent->BindAxis("MoveForward", this, &ACYSPlayer::BindInputMoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ACYSPlayer::BindInputMoveRight);
    PlayerInputComponent->BindAxis("LookUp", this, &ACYSPlayer::BindInputLookUP);
    PlayerInputComponent->BindAxis("Turn", this, &ACYSPlayer::BindInputTurn);
}

void ACYSPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACYSPlayer, ThrottleUp);
    DOREPLIFETIME(ACYSPlayer, ThrottleDown);
    DOREPLIFETIME(ACYSPlayer, PlaneIndex);

    DOREPLIFETIME(ACYSPlayer, cysServerTime);
    DOREPLIFETIME_CONDITION(ACYSPlayer, cysLastInput, COND_SimulatedOnly);
}

float ACYSPlayer::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

    if (GetLocalRole() == ROLE_Authority)
    {
        if (IsLocallyControlled())
        {
            CustomTakeDamage();
        }
        else
        {
            Client_CustomTakeDamage();
        }
    }

    return ActualDamage;
}

void ACYSPlayer::BindInputMoveForward(float Value)
{
    AxisForward = Value;
}

void ACYSPlayer::BindInputMoveRight(float Value)
{
    AxisRight = Value;
}

void ACYSPlayer::BindInputTurn(float Value)
{
    AxisTurn = Value;
}

void ACYSPlayer::BindInputLookUP(float Value)
{
    AxisLookUp = Value;
}

void ACYSPlayer::BindInputFirePressed()
{
    Fire();
    GetWorldTimerManager().SetTimer(TimerHandle_Fire, this, &ACYSPlayer::BindInputFireOn, 1.f, true);
}

void ACYSPlayer::BindInputFireOn()
{
    Fire();

    /*
    FVector Location;
    FRotator Rotation;
    GetController()->GetPlayerViewPoint(Location, Rotation);
    FVector End = Location + (UKismetMathLibrary::GetForwardVector(Rotation) * 50000.f);

    TArray<AActor*> ActorsIgnored;
    ActorsIgnored.Add(this);

    FHitResult Result;
    if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), Location, End, ETraceTypeQuery::TraceTypeQuery_MAX, false, ActorsIgnored, EDrawDebugTrace::Type::ForDuration, Result, true))
    {
        if (Result.bBlockingHit && Result.GetActor())
        {
            if (ACYSPlayer* Player = Cast<ACYSPlayer>(Result.GetActor()))
            {
                
            }
        }
    }
    */
}

void ACYSPlayer::BindInputFireReleased()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_Fire);
}

void ACYSPlayer::Throttle()
{
    if (ThrottleUp)
    {
        CurrentThrottle = FMath::Clamp(CurrentThrottle + 1.f, 0.f, 400.f);
    }
    
    if (ThrottleDown)
    {
        CurrentThrottle = FMath::Clamp(CurrentThrottle + -1.f, 0.f, 400.f);
    }

    if (CurrentThrottle > 50.f)
    {
       // Right_Trail->Activate();
       // Left_Trail->Activate();
    }
    else
    {
        //Right_Trail->Deactivate();
       // Left_Trail->Deactivate();
    }
}

void ACYSPlayer::Speed()
{
    FVector A = CollisionMesh->GetPhysicsLinearVelocity();
    FVector B = CollisionMesh->GetForwardVector() * (CurrentThrottle * 500.f);

    CollisionMesh->SetPhysicsLinearVelocity(UKismetMathLibrary::VLerp(A, B, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()) * 3));
}

void ACYSPlayer::SetInputs(float Forward, float Right, float Turn, float LookUp)
{
    InputForward(Forward);
    InputRight(Right);
    InputTurn(Turn);
    InputLookUp(LookUp);
}

void ACYSPlayer::InputForward(float Value)
{
    if (Value == 0.f)
    {
        ThrottleUp = false;
        ThrottleDown = false;
    }
    else if (Value > 0.f)
    {
        ThrottleUp = true;
        ThrottleDown = false;
    }
    else if (Value < 0.f)
    {
        ThrottleUp = false;
        ThrottleDown = true;
    }
}

void ACYSPlayer::InputRight(float Value)
{
    Rudder = Value * (CurrentThrottle / 750.f);
    float x = (Value * 4000) * (CurrentThrottle / 750.f);

    FVector Torque = UKismetMathLibrary::VLerp(FVector::ZeroVector, CollisionMesh->GetUpVector() * x, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()) * 10);
    CollisionMesh->AddTorqueInDegrees(Torque, NAME_None, true);
}

void ACYSPlayer::InputTurn(float Value)
{
    if (CollisionMesh)
    {
        float A = (Value * -2000);
        float B = A / 3.f;

        FVector Torque = UKismetMathLibrary::VLerp(FVector::ZeroVector, CollisionMesh->GetForwardVector() * UKismetMathLibrary::SelectFloat(A, B, CurrentThrottle > 0.f), UGameplayStatics::GetWorldDeltaSeconds(GetWorld()) * 2);
        CollisionMesh->AddTorqueInDegrees(Torque, NAME_None, true);
    }
}

void ACYSPlayer::InputLookUp(float Value)
{
    if (CollisionMesh)
    {
        float A = (Value * -2000);
        float B = A / 3.f;

        FVector Torque = UKismetMathLibrary::VLerp(FVector::ZeroVector, CollisionMesh->GetRightVector() * UKismetMathLibrary::SelectFloat(A, B, CurrentThrottle > 0.f), UGameplayStatics::GetWorldDeltaSeconds(GetWorld()) * 2);
        CollisionMesh->AddTorqueInDegrees(Torque, NAME_None, true);
    }
}

void ACYSPlayer::UpdateReplication(float DeltaTime)
{
    //Time and Tick
    if (GetNetMode() < ENetMode::NM_Client)
    {
        cysServerTime += DeltaTime;
    }
    if (FMath::Abs(cysServerTime - cysLocalTime) > 0.1f)
    {
        cysLocalTime = cysServerTime;
    }
    else
    {
        cysLocalTime = FMath::Lerp(cysLocalTime, cysServerTime, DeltaTime);
    }

    cysLocalTime += DeltaTime;
    cysTick++;

    UpdateInputs();
    SendInputs(DeltaTime);
}

void ACYSPlayer::UpdateInputs()
{
    if (IsLocallyControlled())
    {
        cysLastInput.Tick = cysTick;
        cysLastInput.AxisForward = AxisForward;
        cysLastInput.AxisRight = AxisRight;
        cysLastInput.AxisTurn = AxisTurn;
        cysLastInput.AxisLookUp = AxisLookUp;

        cysInputBuffer.mBuffer.Add(cysLastInput);
        if (cysInputBuffer.mBuffer.Num() > cysInputBufferSize)
        {
            cysInputBuffer.mBuffer.RemoveAt(0);
        }
    }
    
    if (GetLocalRole() == ROLE_Authority)
    {
        int bufferCount = cysInputBuffer.mBuffer.Num();

        //switch to double consume mode if buffer is twice client buffer
        if (bufferCount >= 2 * cysInputBufferSize)
        {
            cysDoubleConsume = cysInputBufferSize;
        }

        //how many should we consume
        bool canDouble = cysDoubleConsume > 0 && bufferCount > 1 && cysInputBuffer.mBuffer[0].IsSame(cysInputBuffer.mBuffer[1]);
        int serverConsume = canDouble ? 2 : 1;
        if (canDouble) cysDoubleConsume--;

        for (int i = 0; i < serverConsume; i++)
        {
            bool hasInput = bufferCount > 0;

            if (hasInput)
            {
                FInputState s = hasInput ? cysInputBuffer.mBuffer[0] : cysLastInput;
                cysLastInput = s;

                SetInputs(s.AxisForward, s.AxisRight, s.AxisTurn, s.AxisLookUp);
                //mVehicle->getStandardInput()->setInputs(s.mThrottle, s.mBrake, s.mSteer, s.mClutch, s.mGear, s.mHandbrake);

                cysInputBuffer.mBuffer.RemoveAt(0);
            }
        }
    }

    if (GetLocalRole() < ROLE_Authority)
    {
        FInputState s = cysLastInput;
        SetInputs(s.AxisForward, s.AxisRight, s.AxisTurn, s.AxisLookUp);
        //mVehicle->getStandardInput()->setInputs(s.mThrottle, s.mBrake, s.mSteer, s.mClutch, s.mGear, s.mHandbrake);
    }
}

void ACYSPlayer::SendInputs(float DeltaTime)
{
    if (GetLocalRole() < ROLE_Authority)
    {
        cysInputSendTime += DeltaTime;

        float dts = 1.0f / mInputSendRate;
        if (dts < DeltaTime) 
        {
            dts = DeltaTime; //no more than frame rate
        }

        if (cysInputSendTime >= dts)
        {
            cysInputSendTime = 0.0f;
            Server_ReceiveInputs(cysInputBuffer);
        }
    }
}

void ACYSPlayer::ReceiveInput(const FInputState& InputState)
{
    //add & return if empty
    int count = cysInputBuffer.mBuffer.Num();
    if (count == 0)
    {
        cysInputBuffer.mBuffer.Add(InputState);
        return;
    }

    //check existance
    int index = -1;
    for (int i = 0; i < count; i++)
    {
        if (cysInputBuffer.mBuffer[i].Tick == InputState.Tick)
        {
            index = i;
            break;
        }
    }

    //add if non-existant
    if (index == -1)
    {
        //first/last tick
        int ft = cysLastInput.Tick;
        int lt = cysInputBuffer.mBuffer[count - 1].Tick;

        //in-order packet
        if (InputState.Tick == lt + 1)
        {
            cysInputBuffer.mBuffer.Add(InputState);
        }
        //out-of-order packet
        else if (ft < InputState.Tick && InputState.Tick < lt)
        {
            int ni = -1;
            for (int i = 0; i < count; i++)
            {
                ni = i;
                if (cysInputBuffer.mBuffer[i].Tick > InputState.Tick) break;
            }
            if (ni != -1)
            {
                cysInputBuffer.mBuffer.Insert(InputState, ni);
            }
        }
    }
}

void ACYSPlayer::Server_ReceiveInputs_Implementation(const FInputStateBuffer& Inputs)
{
    int count = Inputs.mBuffer.Num();
    for (int i = 0; i < count; i++)
    {
        ReceiveInput(Inputs.mBuffer[i]);
    }
}

void ACYSPlayer::UpdateMesh(int32 Index)
{
    if (Index == 1)
    {
        FSoftObjectPath MeshPath(PlaneMeshPath_1);
        TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(MeshPath, FStreamableDelegate::CreateLambda([this, MeshPath]()
        {
            USkeletalMesh* Mesh = Cast<USkeletalMesh>(MeshPath.TryLoad());
            if (Mesh)
            {
                PlaneMesh->SetSkeletalMesh(Mesh);
                PlaneMesh->SetRelativeLocation(FVector(-1180.f, 0.f, -180.f));
            }
        }));
        
        /*
        auto Mesh = Cast<USkeletalMesh>(StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, PlaneMeshPath_1));
        if (Mesh)
        {
            PlaneMesh->SetSkeletalMesh(Mesh);
            PlaneMesh->SetRelativeLocation(FVector(-1180.f, 0.f, -180.f));
        }
        */
    }
    else if (Index == 2)
    {
        FSoftObjectPath MeshPath(PlaneMeshPath_2);
        TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(MeshPath, FStreamableDelegate::CreateLambda([this, MeshPath]()
        {
            USkeletalMesh* Mesh = Cast<USkeletalMesh>(MeshPath.TryLoad());
            if (Mesh)
            {
                PlaneMesh->SetSkeletalMesh(Mesh);
                PlaneMesh->SetRelativeLocation(FVector(-950.f, 0.f, -180.f));
            }
        }));

        /*
        auto Mesh = Cast<USkeletalMesh>(StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, PlaneMeshPath_2));
        if (Mesh)
        {
            PlaneMesh->SetSkeletalMesh(Mesh);
            PlaneMesh->SetRelativeLocation(FVector(-950.f, 0.f, -180.f));
        }
        */
    }
    else
    {
        FSoftObjectPath MeshPath(PlaneMeshPath_1);
        TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(MeshPath, FStreamableDelegate::CreateLambda([this, MeshPath]()
        {
            USkeletalMesh* Mesh = Cast<USkeletalMesh>(MeshPath.TryLoad());
            if (Mesh)
            {
                PlaneMesh->SetSkeletalMesh(Mesh);
                PlaneMesh->SetRelativeLocation(FVector(-1180.f, 0.f, -180.f));
            }
        }));

        /*
        auto Mesh = Cast<USkeletalMesh>(StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, PlaneMeshPath_1));
        if (Mesh)
        {
            PlaneMesh->SetSkeletalMesh(Mesh);
            PlaneMesh->SetRelativeLocation(FVector(-1180.f, 0.f, -180.f));
        }
        */
    }
}

void ACYSPlayer::Fire()
{
    if (GetLocalRole() < ROLE_Authority)
    {
        Server_Fire();
    }

    FVector Location = FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z - 300.f);
    FRotator Rotation = GetActorRotation();

    FActorSpawnParameters params;
    params.Owner = this;
    params.Instigator = this;
    GetWorld()->SpawnActor<ACYSProjectile>(ACYSProjectile::StaticClass(), FTransform(Rotation, Location), params);
}

void ACYSPlayer::Server_Fire_Implementation()
{
    Fire();
}

void ACYSPlayer::CustomTakeDamage()
{
    if (IsLocallyControlled())
    {
        if (ACYSPlayerController* PC = Cast<ACYSPlayerController>(GetController()))
        {
            if (!TakingDamage)
            {
                PC->ShowDamage();

                TakingDamage = true;
                GetWorldTimerManager().SetTimer(TimerHandle_TakeDamage, this, &ACYSPlayer::CheckTakeDamage, 10.f);
            }
        }
    }
}

void ACYSPlayer::CheckTakeDamage()
{
    if (ACYSPlayerController* PC = Cast<ACYSPlayerController>(GetController()))
    {
        PC->HiddenDamage();

        TakingDamage = false;
        GetWorldTimerManager().ClearTimer(TimerHandle_TakeDamage);
    }
}

void ACYSPlayer::Client_CustomTakeDamage_Implementation()
{
    CustomTakeDamage();
}