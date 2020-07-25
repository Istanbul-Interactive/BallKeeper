// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BallKeeper/Gameplay/Ball/BKBall.h"
#include "BKCharacter.generated.h"

UCLASS()
class BALLKEEPER_API ABKCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABKCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		class UCameraComponent* CameraComponent;
	UPROPERTY(EditAnywhere, Category = "Camera")
		FName HeadSocketName;

	//This is where the grabbed object is going to stick at.
	UPROPERTY(EditDefaultsOnly, Category = "Gravity Gun")
		class USceneComponent* ObjectCarryPoint;
	UPROPERTY(EditDefaultsOnly, Category = "Gravity Gun")
		float ShootStrength;
	UPROPERTY(EditDefaultsOnly, Category = "Gravity Gun")
		float ReachDistance;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Team")
		int TeamId;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Server, Reliable ,BlueprintCallable, Category = "BallKeeper|Player")
		void SpawnPlayer();

protected:
	UPROPERTY()
		AActor* GrabbedObject;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float value);
	void MoveRight(float value);
	void LookUp(float value);
	void Turn(float value);

	//Fires the gravity gun
	void StartFire();
	void StopFire();

	UFUNCTION(Server, Reliable)
		void ServerGrabObject();
	UFUNCTION(Server, Reliable)
		void ServerThrowObject(FVector ClientForwardVector);
};
