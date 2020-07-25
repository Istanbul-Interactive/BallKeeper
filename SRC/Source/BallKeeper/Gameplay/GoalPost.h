// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "GoalPost.generated.h"

UCLASS()
class BALLKEEPER_API AGoalPost : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGoalPost();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
		UBoxComponent* BoxComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
