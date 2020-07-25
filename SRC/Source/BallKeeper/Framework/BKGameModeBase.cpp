// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#include "BKGameModeBase.h"

#include "BallKeeper/Gameplay/Player/BKCharacter.h"
#include "Kismet/GameplayStatics.h"

ABKGameModeBase::ABKGameModeBase()
{
}

void ABKGameModeBase::SpawnBall(FVector Location, FRotator Rotation) const
{
	if (BallToSpawn)
	{
		FActorSpawnParameters SpawnParams;

		GetWorld()->SpawnActor<ABKBall>(BallToSpawn, Location, Rotation, SpawnParams);
	}
}

void ABKGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	ConnectedPlayers.Add(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	SpawnBall(FVector(0.0f, 0.0f, 400.0f), FRotator(0.0f, 0.0f, 0.0f));
}

void ABKGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ABKGameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	UE_LOG(LogTemp, Warning, TEXT("Player joining..."));

	if (GetNumPlayers() >= 15)
	{
		UE_LOG(LogTemp, Error, TEXT("Max player count reached. Refusing connection..."));
		ErrorMessage = TEXT("Server is full!");
	}
}

void ABKGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UE_LOG(LogTemp, Warning, TEXT("Player joined!"));
	ConnectedPlayers.Add(NewPlayer);

	
	ABKCharacter* NewCharacter = Cast<ABKCharacter>(NewPlayer->GetPawn());

	if(NewCharacter)
	{
		int TeamOne = 0;
		int TeamTwo = 0;
		
		for(int i = 0; i < ConnectedPlayers.Num(); i++)
		{
			ABKCharacter* player = Cast<ABKCharacter>(ConnectedPlayers[i]->GetPawn());
			if (player->TeamId == 1)
				TeamOne++;
			else
				TeamTwo++;
		}

		if (TeamOne < TeamTwo)
			NewCharacter->TeamId = 1;
		else if (TeamTwo < TeamOne)
			NewCharacter->TeamId = 2;
		else
		{
			NewCharacter->TeamId = FMath::RandRange(1, 2);
		}

		UE_LOG(LogTemp, Warning, TEXT("Player initialization completed!"));

		NewCharacter->SpawnPlayer();
	}
}

void ABKGameModeBase::Logout(AController* Exiting)
{
	ConnectedPlayers.Remove(Cast<APlayerController>(Exiting));
}