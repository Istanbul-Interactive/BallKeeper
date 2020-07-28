// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BallKeeper/Gameplay/Ball/BKBall.h"
#include "BallKeeper/Gameplay/Player/BKCharacter.h"
#include "BallKeeper/Gameplay/Player/BKSpectatorPawn.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "GameFramework/GameSession.h"
#include "BKGameModeBase.generated.h"

UCLASS()
class BALLKEEPER_API ABKGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	TArray<APlayerController*> ConnectedPlayers;

	UPROPERTY(EditAnywhere, Category = "Spawn")
		TSubclassOf<ABKBall> BallToSpawn;

	UPROPERTY(EditAnywhere, Category = "Spawn")
		TSubclassOf<ABKSpectatorPawn> PlayerSpectator;

	UPROPERTY(EditAnywhere, Category = "Spawn")
		TSubclassOf<ABKCharacter> PlayerToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
		FVector TeamOneSpawnPointLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
		FRotator TeamOneSpawnPointRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
		FVector TeamTwoSpawnPointLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
		FRotator TeamTwoSpawnPointRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
		int MaxPlayerCount;

public:
	ABKGameModeBase();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

public:
	UFUNCTION(BlueprintCallable, Category = "BallKeeper|Ball")
		void SpawnBall(FVector Location, FRotator Rotation) const;
	
	UFUNCTION(Server, Reliable, Category = "BallKeeper|Teams")
		void CheckRestartCondition();

	UFUNCTION(Server, Reliable, BlueprintCallable ,Category = "BallKeeper|Teams")
		void KillAllPlayers();

	UFUNCTION(Category = "BallKeeper|Teams")
		int GetTeamOnePlayerCount();

	UFUNCTION(Category = "BallKeeper|Teams")
		int GetTeamTwoPlayerCount();
};
