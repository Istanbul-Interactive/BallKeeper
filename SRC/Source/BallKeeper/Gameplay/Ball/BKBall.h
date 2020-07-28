// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
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

	//For physics collision detection
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USphereComponent* SphereTrigger;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		bool IsGrabbed;

	//The last team to carry the ball
	UPROPERTY()
		int LastTeamId;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BallKeeper|Ball")
		void ResetBallLocation();

	UFUNCTION(BlueprintCallable, Category = "BallKeeper|Ball")
		void OnCollisionHit(AActor* OtherActor) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "BallKeeper|Ball")
		void OnBallReset();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	FVector BallResetLocation;
};
