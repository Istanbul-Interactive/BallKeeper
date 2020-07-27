// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#include "BKGameModeBase.h"

#include "BallKeeper/Gameplay/Player/BKCharacter.h"
#include "Kismet/GameplayStatics.h"

ABKGameModeBase::ABKGameModeBase()
{
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

	AssignPlayerTeam(NewPlayer);
}

void ABKGameModeBase::Logout(AController* Exiting)
{
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
		ABKCharacter* Character = Cast<ABKCharacter>(ConnectedPlayers[i]->GetCharacter());
		if (Character != nullptr && Character->TeamId == 1)
		{
			TeamOneCount++;
		}
	}

	return TeamOneCount;
}

int ABKGameModeBase::GetTeamTwoPlayerCount()
{
	int TeamTwoCount = 0;

	for (int i = 0; i < ConnectedPlayers.Num(); i++)
	{
		ABKCharacter* Character = Cast<ABKCharacter>(ConnectedPlayers[i]->GetCharacter());
		if (Character != nullptr && Character->TeamId == 2)
		{
			TeamTwoCount++;
		}
	}

	return TeamTwoCount;
}

void ABKGameModeBase::RestartGame_Implementation()
{
	for (int i = 0; i < ConnectedPlayers.Num(); i++)
	{
		int TeamOneCount = 0;
		int TeamTwoCount = 0;

		ABKCharacter* Character = Cast<ABKCharacter>(ConnectedPlayers[i]->GetCharacter());
		//Destroying all ABKCharacters in game to reset everything.

		if (Character != nullptr)
		{
			ConnectedPlayers[i]->UnPossess();
			Character->Destroy();
		}

		ABKCharacter* NewCharacter = GetWorld()->SpawnActor<ABKCharacter>(PlayerToSpawn, FVector(0.0f, 0.0f, 0.0f), FRotator(0.0f, 0.0f, 0.0));
		ConnectedPlayers[i]->Possess(NewCharacter);
		AssignPlayerTeam(ConnectedPlayers[i]);

		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABKBall::StaticClass(), OUT FoundActors);

		ABKBall* Ball = Cast<ABKBall>(FoundActors[0]);
		if (Ball != nullptr)
			Ball->ResetBallLocation();
	}
}

void ABKGameModeBase::AssignPlayerTeam_Implementation(APlayerController* NewPlayer)
{
	ABKCharacter* NewCharacter = Cast<ABKCharacter>(NewPlayer->GetCharacter());

	if (NewCharacter)
	{
		int TeamOne = GetTeamOnePlayerCount();
		int TeamTwo = GetTeamTwoPlayerCount();

		if (TeamOne < TeamTwo)
			NewCharacter->TeamId = 1;
		else if (TeamTwo < TeamOne)
			NewCharacter->TeamId = 2;
		else
		{
			NewCharacter->TeamId = FMath::RandRange(1, 2);
		}

		NewCharacter->ResetPlayerPosition();
	}
}

void ABKGameModeBase::OnPlayerDeath_Implementation()
{
	int TeamOneRemainingCount = GetTeamOnePlayerCount();
	int TeamTwoRemainingCount = GetTeamTwoPlayerCount();

	if (ConnectedPlayers.Num() > 1)
	{
		if (TeamOneRemainingCount == 0 || TeamTwoRemainingCount == 0)
			RestartGame();
	}
}

void ABKGameModeBase::PlayerDeath_Implementation(ABKCharacter* Character)
{
	//Storing the Character's position and rotation in memory.
	const FVector PlayerLocation = Character->GetActorLocation();
	const FRotator PlayerRotation = Character->GetActorRotation();
	const FActorSpawnParameters SpawnParams;

	APlayerController* PC = Cast<APlayerController>(Character->GetController());

	//Spawning the new spectator into the game world.
	ABKSpectatorPawn* NewSpectator = GetWorld()->SpawnActor<ABKSpectatorPawn>(PlayerSpectator, PlayerLocation, PlayerRotation, SpawnParams);
	PC->UnPossess();
	APawn* PawnToPosses = Cast<APawn>(NewSpectator);
	//Making the player posses the newly created spectator pawn.
	PC->Possess(PawnToPosses);
	//Destroying the dead player from the game scene.
	Character->Destroy();

	OnPlayerDeath();
}