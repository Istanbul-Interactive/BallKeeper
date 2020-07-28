// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#include "BKPlayerControllerBase.h"
#include "BallKeeper/Framework/BKGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ABKPlayerControllerBase::ABKPlayerControllerBase()
{
}

void ABKPlayerControllerBase::AssignTeam_Implementation()
{
	const int TeamOne = GetGameMode()->GetTeamOnePlayerCount();
	const int TeamTwo = GetGameMode()->GetTeamTwoPlayerCount();

	ABKCharacter* Char = Cast<ABKCharacter>(GetCharacter());
	
	if (TeamOne < TeamTwo)
	{
		TeamId = 1;
		Char->TeamId = TeamId;
	}
	else if (TeamTwo < TeamOne)
	{
		TeamId = 2;
		Char->TeamId = TeamId;
	}
	else if (TeamOne == TeamTwo)
	{
		TeamId = FMath::RandRange(1, 2);
		Char->TeamId = TeamId;
	}
}

void ABKPlayerControllerBase::KillPlayer_Implementation()
{
	ABKCharacter* CharacterToKill = Cast<ABKCharacter>(GetCharacter());

	if (CharacterToKill != nullptr)
	{
		//Storing Character's vector and rotation to pass as a parameter.
		FVector Position = CharacterToKill->GetActorLocation();
		FRotator Rotation = CharacterToKill->GetActorRotation();

		UnPossess();
		CharacterToKill->Destroy();

		MakePlayerSpectator(Position, Rotation);
		OnDeath();
		TeamId = 0;
		GetGameMode()->CheckRestartCondition();
	}
}

void ABKPlayerControllerBase::MakePlayerSpectator_Implementation(const FVector Location, const FRotator Rotation)
{
	const FActorSpawnParameters SpawnParams;

	ABKSpectatorPawn* NewSpectator = GetWorld()->SpawnActor<ABKSpectatorPawn>(PlayerSpectator, Location, Rotation, SpawnParams);
	Possess(NewSpectator);
}

void ABKPlayerControllerBase::SpawnCharacter_Implementation(TSubclassOf<ABKCharacter> PlayerCharacter)
{
	APawn* PawnToUnPossess = GetPawn();
	if (PawnToUnPossess != nullptr)
	{
		UnPossess();
		PawnToUnPossess->Destroy();
	}

	ABKCharacter* NewCharacter = GetWorld()->SpawnActor<ABKCharacter>(PlayerCharacter, FVector(0.0f, 0.0f, 0.0f), FRotator(0.0f, 0.0f, 0.0f));
	if (NewCharacter != nullptr)
	{
		Possess(NewCharacter);
		AssignTeam();
		NewCharacter->ResetPlayerPosition();
		OnSpawn();
	}
	else
		UE_LOG(LogTemp, Error, TEXT("New Character NULLPTR!!!"));
}

void ABKPlayerControllerBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicates to everyone
	DOREPLIFETIME(ABKPlayerControllerBase, TeamId);
}

ABKGameModeBase* ABKPlayerControllerBase::GetGameMode() const
{
	return Cast<ABKGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
}