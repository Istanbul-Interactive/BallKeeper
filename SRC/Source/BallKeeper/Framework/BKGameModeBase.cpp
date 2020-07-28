// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#include "BKGameModeBase.h"

#include "BallKeeper/Gameplay/Player/BKPlayerControllerBase.h"
#include "Kismet/GameplayStatics.h"

ABKGameModeBase::ABKGameModeBase()
{
}

void ABKGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	/*ConnectedPlayers.Add(UGameplayStatics::GetPlayerController(GetWorld(), 0));*/

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

	if (GetNumPlayers() >= MaxPlayerCount)
	{
		UE_LOG(LogTemp, Error, TEXT("Max player count reached. Refusing connection..."));
		ErrorMessage = TEXT("Server is full!");
	}
}

void ABKGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	UE_LOG(LogTemp, Warning, TEXT("Player joined!"));

	UE_LOG(LogTemp, Warning, TEXT("Array added!"));
	ConnectedPlayers.Add(NewPlayer);

	ABKPlayerControllerBase* PC = Cast<ABKPlayerControllerBase>(NewPlayer);
	if (PC)
		PC->SpawnCharacter(PlayerToSpawn);
}

void ABKGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	UE_LOG(LogTemp, Warning, TEXT("Array Removed"));
	ConnectedPlayers.Remove(Cast<APlayerController>(Exiting));
}

void ABKGameModeBase::SpawnBall(FVector Location, FRotator Rotation) const
{
	if (BallToSpawn)
	{
		FActorSpawnParameters SpawnParams;
		ABKBall* spawnedBall = GetWorld()->SpawnActor<ABKBall>(BallToSpawn, Location, Rotation, SpawnParams);
		spawnedBall->ResetBallLocation();
	}
}

int ABKGameModeBase::GetTeamOnePlayerCount()
{
	int TeamOneCount = 0;

	for (int i = 0; i < ConnectedPlayers.Num(); i++)
	{
		ABKPlayerControllerBase* PC = Cast<ABKPlayerControllerBase>(ConnectedPlayers[i]);
		if (PC != nullptr && PC->TeamId == 1)
		{
			TeamOneCount++;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Team One: %i"), TeamOneCount);

	return TeamOneCount;
}

int ABKGameModeBase::GetTeamTwoPlayerCount()
{
	int TeamTwoCount = 0;

	for (int i = 0; i < ConnectedPlayers.Num(); i++)
	{
		ABKPlayerControllerBase* PC = Cast<ABKPlayerControllerBase>(ConnectedPlayers[i]);
		if (PC != nullptr && PC->TeamId == 2)
		{
			TeamTwoCount++;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Team Two %i"), TeamTwoCount);

	return TeamTwoCount;
}

void ABKGameModeBase::KillAllPlayers_Implementation()
{
	for (int i = 0; i < ConnectedPlayers.Num(); i++)
	{
		ACharacter* Char = ConnectedPlayers[i]->GetCharacter();
		if (Char != nullptr)
		{
			ConnectedPlayers[i]->UnPossess();
			Char->Destroy();
		}
	}
}

void ABKGameModeBase::CheckRestartCondition_Implementation()
{
	//Checking if either one of the teams have won.
	const int TeamOneRemainingCount = GetTeamOnePlayerCount();
	const int TeamTwoRemainingCount = GetTeamTwoPlayerCount();

	if (TeamOneRemainingCount == 0 || TeamTwoRemainingCount == 0)
	{
		KillAllPlayers();

		for (int i = 0; i < ConnectedPlayers.Num(); i++)
		{
			ABKPlayerControllerBase* PC = Cast<ABKPlayerControllerBase>(ConnectedPlayers[i]);
			PC->SpawnCharacter(PlayerToSpawn);

			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABKBall::StaticClass(), OUT FoundActors);

			ABKBall* Ball = Cast<ABKBall>(FoundActors[0]);
			if (Ball != nullptr)
				Ball->ResetBallLocation();
		}
	}
}