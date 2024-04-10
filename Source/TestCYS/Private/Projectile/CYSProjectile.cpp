// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/CYSProjectile.h"

//ENGINE
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/GameplayStatics.h"

ACYSProjectile::ACYSProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

    Collision = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Collision"));
    Collision->SetSimulatePhysics(true);
    Collision->SetEnableGravity(false);
    Collision->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
    Collision->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
    Collision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    Collision->SetHiddenInGame(true);
    RootComponent = Collision;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CuboMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
    if (CuboMeshAsset.Succeeded())
    {
        Collision->SetStaticMesh(CuboMeshAsset.Object);
        Collision->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    }

    Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
    Mesh->SetGenerateOverlapEvents(true);
    Mesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
    Mesh->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
    Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    Mesh->SetupAttachment(Collision);

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("SkeletalMesh'/Game/Assets/VigilanteContent/Projectiles/West_Missile_AGM88/SK_West_Missile_AGM88.SK_West_Missile_AGM88'"));
    if (MeshAsset.Succeeded())
    {
        Mesh->SetSkeletalMesh(MeshAsset.Object);
        Mesh->SetRelativeLocation(FVector(-200.0f, 0.0f, 0.0f));
    }

    Muzzle = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Muzzle"));
    Muzzle->SetupAttachment(Mesh);

    static ConstructorHelpers::FObjectFinder<UNiagaraSystem> NiagaraAsset(TEXT("NiagaraSystem'/Game/Assets/VigilanteContent/Projectiles/West_Missile_AGM88/FX/NS_West_Missile_AMG88.NS_West_Missile_AMG88'"));
    if (NiagaraAsset.Succeeded())
    {
        Muzzle->SetAsset(NiagaraAsset.Object);
        Muzzle->SetRelativeLocation(FVector(-200.0f, 0.0f, 0.0f));
    }

    Mesh->OnComponentBeginOverlap.AddDynamic(this, &ACYSProjectile::OnOverlapBegin);
    Mesh->OnComponentEndOverlap.AddDynamic(this, &ACYSProjectile::OnOverlapEnd);

    bReplicates = true;
    SetReplicateMovement(true);
    bNetUseOwnerRelevancy = true;
}

void ACYSProjectile::BeginPlay()
{
	Super::BeginPlay();

    if (GetLocalRole() == ROLE_Authority)
    {
	    SetLifeSpan(5.f);
    }
}

void ACYSProjectile::Destroyed()
{
    Super::Destroyed();
    //UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), nullptr, GetActorTransform());
    UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Destroyed"));
}

void ACYSProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (GetLocalRole() == ROLE_Authority)
    {
        FVector Torque = UKismetMathLibrary::NegateVector(Collision->GetPhysicsAngularVelocityInDegrees()) / 0.75f;
        Collision->AddTorqueInDegrees(Torque, NAME_None, true);
        Collision->AddForce(FVector(0, 0, -1000.f), NAME_None, true);

        Speed();
    }
}

void ACYSProjectile::Speed()
{
    FVector A = Collision->GetPhysicsLinearVelocity();
    FVector B = Collision->GetForwardVector() * (100.f * 500.f);

    Collision->SetPhysicsLinearVelocity(UKismetMathLibrary::VLerp(A, B, 0.1f));
}

void ACYSProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (GetLocalRole() == ROLE_Authority)
    {
        UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Hit: %s | Owner: %s"), *OtherActor->GetName(), *GetOwner()->GetName()));

        if (OtherActor != GetOwner())
        {
            TArray<AActor*> IgnoredActors;
            IgnoredActors.Add(GetOwner());
            if (UGameplayStatics::ApplyRadialDamage(GetWorld(), 1000.f, GetActorLocation(), 500.f, UDamageType::StaticClass(), IgnoredActors, GetOwner(), nullptr, false, ECollisionChannel::ECC_Visibility))
            {
                Destroy();
            }
        }
    }
}

void ACYSProjectile::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}