// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#include "BKBall.h"

#include "BallKeeper/Framework/BKGameModeBase.h"
#include "BallKeeper/Gameplay/Player/BKCharacter.h"
#include "BallKeeper/Gameplay/Player/BKPlayerControllerBase.h"

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

	if (HasAuthority() && GetActorLocation().Z <= -1000.0f)
		ResetBallLocation();

	//UE_LOG(LogTemp, Warning, TEXT("Last Team ID: %i"), LastTeamId);
}

void ABKBall::ResetBallLocation_Implementation()
{
	BallMesh->SetAllPhysicsLinearVelocity(FVector(0.0f, 0.0f, 0.0f), false);
	BallMesh->SetSimulatePhysics(false);
	LastTeamId = 0;
	SetActorLocation(BallResetLocation);

	OnBallReset();
}

void ABKBall::OnCollisionHit(AActor* OtherActor) const
{
	//Making sure that the code only runs on server.
	if (GetNetMode() == ENetMode::NM_ListenServer)
	{
		//Team ID 0 = no team ever claimed the ball. It's neutral.
		if (LastTeamId == 0)
			return;

		ABKCharacter* Character = Cast<ABKCharacter>(OtherActor);
		ABKPlayerControllerBase* PC;
		
		if (Character != nullptr)
			PC = Cast<ABKPlayerControllerBase>(Character->Controller);
		else
			PC = nullptr;

		//Checking if the last team to control the ball is same as the hit character's team.
		if (PC != nullptr && LastTeamId != PC->TeamId)
		{
			ABKGameModeBase* GM = Cast<ABKGameModeBase>(GetWorld()->GetAuthGameMode());
			PC->KillPlayer();
		}
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