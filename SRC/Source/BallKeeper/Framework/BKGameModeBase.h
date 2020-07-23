// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BallKeeper/Gameplay/Ball/BKBall.h"
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

	UPROPERTY(EditAnywhere, Category = "Spawning")
		TSubclassOf<ABKBall> BallToSpawn;

public:
	ABKGameModeBase();

	UFUNCTION(BlueprintCallable, Category = "BallKeeper|Ball")
		void SpawnBall(FVector Location, FRotator Rotation) const;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;
};
