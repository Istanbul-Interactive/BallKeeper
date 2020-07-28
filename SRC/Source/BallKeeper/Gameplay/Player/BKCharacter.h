// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/Character.h"
#include "BKCharacter.generated.h"

class ABKPlayerControllerBase;
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
		USpotLightComponent* SpotLight;

	//This is modified from Player Controller! Don't change this from Character!
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Character")
		int TeamId;
	
	//This is where the grabbed object is going to stick at.
	UPROPERTY(EditDefaultsOnly, Category = "Gravity Gun")
		class USceneComponent* ObjectCarryPoint;

	UPROPERTY(EditDefaultsOnly, Category = "Gravity Gun")
		float ShootStrength;
	
	UPROPERTY(EditDefaultsOnly, Category = "Gravity Gun")
		float ReachDistance;

	UPROPERTY(EditDefaultsOnly, Category = "Character")
		float DashPower;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Character")
		bool IsCarryingBall;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BallKeeper|Player")
		void ResetPlayerPosition();

	UFUNCTION(BlueprintImplementableEvent, Category = "BallKeeper|Ball")
		void OnBallCarry();

	UFUNCTION(BlueprintImplementableEvent, Category = "BallKeeper|Ball")
		void OnBallDrop();

	APlayerController* GetPlayerController() const;

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
	void ClientDashForward();
	void ClientDashBackward();
	void ClientDashRight();
	void ClientDashLeft();

	UFUNCTION(Server, Reliable)
		void ServerDashForward();
	UFUNCTION(Server, Reliable)
		void ServerDashBackward();
	UFUNCTION(Server, Reliable)
		void ServerDashRight();
	UFUNCTION(Server, Reliable)
		void ServerDashLeft();

	//Fires the gravity gun
	void StartFire();
	void StopFire();

	UFUNCTION(Server, Reliable)
		void ServerGrabObject();
	UFUNCTION(Server, Reliable)
		void ServerThrowObject(const FVector ClientForwardVector);
};
