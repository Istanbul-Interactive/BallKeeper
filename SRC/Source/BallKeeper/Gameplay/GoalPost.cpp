// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.


#include "GoalPost.h"

// Sets default values
AGoalPost::AGoalPost()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>("Box Collision");
	RootComponent = BoxComponent;

	BoxComponent->SetCollisionProfileName(FName("OverlapAll"));
	//BoxComponent->MoveIgnoreActors.Add(this);
	
}

// Called when the game starts or when spawned
void AGoalPost::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGoalPost::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

