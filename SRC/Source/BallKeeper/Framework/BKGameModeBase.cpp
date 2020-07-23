// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#include "BKGameModeBase.h"
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

	if (GetNumPlayers() >= 2)
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
}

void ABKGameModeBase::Logout(AController* Exiting)
{
	ConnectedPlayers.Remove(Cast<APlayerController>(Exiting));
}