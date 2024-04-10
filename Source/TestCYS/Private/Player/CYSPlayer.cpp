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
#include "GameFramework/PawnMovementComponent.h"

// Sets default values
ACYSPlayer::ACYSPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

    CollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Collision Mesh"));
    CollisionMesh->SetGenerateOverlapEvents(true);
    //CollisionMesh->SetSimulatePhysics(false);
    //CollisionMesh->SetEnableGravity(false);
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
        TargetSpeed = MinSpeed;
        CurrentSpeed = MinSpeed;

        if (ACYSPlayerController* PC = Cast<ACYSPlayerController>(GetController()))
        {
            PC->bShowMouseCursor = false;
            PC->SetInputMode(FInputModeGameOnly());
        }

        UWidgetLayoutLibrary::RemoveAllWidgets(GetWorld());
    }

    UpdateMesh(PlaneIndex);
}

void ACYSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (IsLocallyControlled())
    {
        ForwardMovement(DeltaTime);
        RotationMovement(DeltaTime);

        Server_UpdateData(Yaw, Pitch, Roll, GetActorLocation(), GetActorRotation());
    }
    else
    {
        SetClientTransform();
    }
}

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

    DOREPLIFETIME(ACYSPlayer, PlaneIndex);

    DOREPLIFETIME(ACYSPlayer, YawRep);
    DOREPLIFETIME(ACYSPlayer, PitchRep);
    DOREPLIFETIME(ACYSPlayer, RollRep);
    DOREPLIFETIME(ACYSPlayer, ActorLocation);
    DOREPLIFETIME(ACYSPlayer, ActorRotation);
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
    if (IsLocallyControlled())
    {
        if (UKismetMathLibrary::Abs(Value) > 0.1f)
        {
            TargetSpeed = UKismetMathLibrary::Clamp(UKismetMathLibrary::FInterpTo_Constant(TargetSpeed, Value * MaxSpeed, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), SpeedInterp), MinSpeed, MaxSpeed);
        }

        float Value1 = ((CurrentSpeed - TargetSpeed) / MaxSpeed);
        float Value2 = UKismetMathLibrary::Abs(Value1) + 0.2f;
        float Interp = Value2 * SpeedInterp;
        CurrentSpeed = UKismetMathLibrary::FInterpTo_Constant(CurrentSpeed, TargetSpeed, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), Interp);
        Acceleration = (TargetSpeed >= CurrentSpeed) ? 0.f : -1.f;
    }
}

void ACYSPlayer::BindInputMoveRight(float Value)
{
    Yaw = Value;
    DeltaYaw = UKismetMathLibrary::Clamp(UKismetMathLibrary::FInterpTo_Constant(DeltaYaw, Yaw * YawMax, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), UKismetMathLibrary::Abs(DeltaYaw) + InterpYaw), -YawMax, YawMax);
}

void ACYSPlayer::BindInputTurn(float Value)
{
    Roll = Value;
    DeltaRoll = UKismetMathLibrary::Clamp(UKismetMathLibrary::FInterpTo_Constant(DeltaRoll, Roll * RollMax, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), UKismetMathLibrary::Abs(DeltaRoll) + InterpRoll), -RollMax, RollMax);
}

void ACYSPlayer::BindInputLookUP(float Value)
{
    Pitch = Value;
    DeltaPitch = UKismetMathLibrary::Clamp(UKismetMathLibrary::FInterpTo_Constant(DeltaPitch, Pitch * PitchMax, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), UKismetMathLibrary::Abs(DeltaPitch) + InterpPitch), -PitchMax, PitchMax);
}

void ACYSPlayer::BindInputFirePressed()
{
    Fire();
    GetWorldTimerManager().SetTimer(TimerHandle_Fire, this, &ACYSPlayer::BindInputFireOn, 1.f, true);
}

void ACYSPlayer::BindInputFireOn()
{
    Fire();
}

void ACYSPlayer::BindInputFireReleased()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_Fire);
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

void ACYSPlayer::ForwardMovement(float DeltaTime)
{
    AddActorLocalOffset(FVector(DeltaTime * CurrentSpeed, 0.f, 0.f), true);
}

void ACYSPlayer::RotationMovement(float DeltaTime)
{
    FRotator DeltaRot = FRotator(DeltaTime * DeltaPitch, DeltaTime * DeltaYaw, DeltaTime * DeltaRoll);
    AddActorLocalRotation(DeltaRot, true);
}

void ACYSPlayer::SetClientTransform()
{
    SetActorRotation(UKismetMathLibrary::RInterpTo(GetActorRotation(), ActorRotation, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), 8.0f));
    SetActorLocation(UKismetMathLibrary::VInterpTo(GetActorLocation(), ActorLocation, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), 8.0f));
}

void ACYSPlayer::Server_UpdateData_Implementation(float CustomYaw, float CustomPitch, float CustomRoll, FVector CustomLocation, FRotator CustomRotation)
{
    YawRep = CustomYaw;
    PitchRep = CustomPitch;
    RollRep = CustomRoll;
    ActorLocation = CustomLocation;
    ActorRotation = CustomRotation;
}