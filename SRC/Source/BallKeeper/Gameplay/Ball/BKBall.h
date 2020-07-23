// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BKBall.generated.h"

UCLASS()
class BALLKEEPER_API ABKBall : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABKBall();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball")
		UStaticMeshComponent* BallMesh;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		bool IsGrabbed;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BallKeeper|Ball")
		void ResetBallLocation();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
		void DropBall();

protected:
	FVector BallResetLocation;
};
