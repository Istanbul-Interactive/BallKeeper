// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#include "BKBall.h"

#include "BallKeeper/Gameplay/Player/BKCharacter.h"

// Sets default values
ABKBall::ABKBall()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>("Static Mesh Component");
	SetRootComponent(BallMesh);
	SphereTrigger = CreateDefaultSubobject<USphereComponent>("Sphere Trigger");
	SphereTrigger->SetupAttachment(BallMesh);
}

// Called every frame
void ABKBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector pos = GetActorLocation();
	if (pos.Z <= -1000.0f && HasAuthority())
		ResetBallLocation();
}

void ABKBall::ResetBallLocation_Implementation()
{
	BallMesh->SetAllPhysicsLinearVelocity(FVector(0.0f, 0.0f, 0.0f), false);
	SetActorLocation(BallResetLocation);
}

void ABKBall::OnCollisionHit(AActor* OtherActor)
{
	ABKCharacter* Character = Cast<ABKCharacter>(OtherActor);
	UE_LOG(LogTemp, Warning, TEXT("Hit "));

	if(Character && LastTeamId != Character->TeamId)
	{
		Character->Destroy();
	}
}

// Called when the game starts or when spawned
void ABKBall::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() == ENetMode::NM_ListenServer)
	{
		BallResetLocation = GetActorLocation();
		SetReplicates(true);
		SetReplicateMovement(true);
		BallMesh->SetSimulatePhysics(true);
	}
}

void ABKBall::DropBall_Implementation()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	BallMesh->SetSimulatePhysics(true);
}
