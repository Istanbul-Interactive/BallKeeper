// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BallKeeper/Framework/BKGameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "BKPlayerControllerBase.generated.h"

/**
 *
 */
UCLASS()
class BALLKEEPER_API ABKPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Team")
		int TeamId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
		TSubclassOf<ABKSpectatorPawn> PlayerSpectator;

public:
	ABKPlayerControllerBase();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BallKeeper|Player")
		void AssignTeam();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BallKeeper|Player")
		void KillPlayer();

	//Blueprint Implementation
	UFUNCTION(BlueprintImplementableEvent, Category = "BallKeeper|Player")
		void OnDeath();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BallKeeper|Player")
		void MakePlayerSpectator(FVector Location, FRotator Rotation);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BallKeeper|Player")
		void SpawnCharacter(TSubclassOf<ABKCharacter> PlayerCharacter);

	//Blueprint Implementation
	UFUNCTION(BlueprintImplementableEvent, Category = "BallKeeper|Player")
		void OnSpawn();

protected:
	ABKGameModeBase* GetGameMode() const;
};
