// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#include "BKCharacter.h"

#include "DrawDebugHelpers.h"
#include "BallKeeper/Framework/BKGameModeBase.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#define OUT

// Sets default values
ABKCharacter::ABKCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicates(true);

	HeadSocketName = TEXT("headSocket");
	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(GetMesh(), HeadSocketName);
	CameraComponent->bUsePawnControlRotation = true;

	ObjectCarryPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ObjectCarryPoint"));
	ObjectCarryPoint->SetupAttachment(CameraComponent);

	ReachDistance = 1000.0f;
}

// Called every frame
void ABKCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ABKCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABKCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABKCharacter::StopFire);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABKCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABKCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ABKCharacter::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &ABKCharacter::Turn);
}



//Called from BKGameModeBase
void ABKCharacter::ResetPlayerPosition_Implementation()
{
	ABKGameModeBase* GM = Cast<ABKGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	//Spawning player according to their team spawn point.
	if (TeamId == 1)
	{
		SetActorLocation(GM->TeamOneSpawnPointLocation);
		SetActorRotation(GM->TeamOneSpawnPointRotation);
	}
	else if (TeamId == 2)
	{
		SetActorLocation(GM->TeamTwoSpawnPointLocation);
		SetActorRotation(GM->TeamTwoSpawnPointRotation);
	}
}

void ABKCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicates to everyone
	DOREPLIFETIME(ABKCharacter, TeamId);
	DOREPLIFETIME(ABKCharacter, IsCarryingBall);
}

// Called when the game starts or when spawned
void ABKCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ABKCharacter::MoveForward(float value)
{
	if (value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), value);
	}
}

void ABKCharacter::MoveRight(float value)
{
	if (value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), value / 1.2f);
	}
}

void ABKCharacter::LookUp(float value)
{
	if (value != 0.0f)
	{
		AddControllerPitchInput(value);
	}
}

void ABKCharacter::Turn(float value)
{
	if (value != 0.0f)
	{
		AddControllerYawInput(value);
	}
}

void ABKCharacter::StartFire()
{
	ServerGrabObject();
}

void ABKCharacter::ServerGrabObject_Implementation()
{
	const FCollisionQueryParams QueryParams("ObjectCarryTrace", false, this);

	//Casting the controller to APlayerController class.
	APlayerController* PC = Cast<APlayerController>(this->GetController());

	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(OUT CameraLocation, OUT CameraRotation);

	const FVector Start = CameraLocation;
	const FVector ForwardVector = CameraLocation.ForwardVector;
	const FVector End = CameraLocation + (CameraRotation.Vector() * ReachDistance);
	FHitResult Hit;

	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams))
		return;

	UPrimitiveComponent* PrimComp = Hit.GetComponent();

	//To prevent trying to pick up any random object.
	if (Hit.GetActor()->IsA<ABKBall>())
	{
		//GrabbedObject is ABKBall
		GrabbedObject = Hit.GetActor();

		ABKBall* ball = Cast<ABKBall>(GrabbedObject);
		//If ball is casted successfully than we grabbed a ball.
		if (ball != nullptr)
		{
			ball->IsGrabbed = true;
			ball->LastTeamId = TeamId;
			ball->SetActorEnableCollision(false);
		}

		//RootComponent of the ABKBall class is it's static mesh.
		UStaticMeshComponent* ObjectMesh = Cast<UStaticMeshComponent>(GrabbedObject->GetRootComponent());
		if (ObjectMesh != nullptr)
		{
			ObjectMesh->SetSimulatePhysics(false);
		}

		//Attaching ABKBall to ABKCharacter's ObjectCarryPoint.
		GrabbedObject->AttachToComponent(this->ObjectCarryPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeadSocketName);
		OnBallCarry();
		IsCarryingBall = true;
	}
}

void ABKCharacter::StopFire()
{
	//Feeding the ServerThrowObject with Camera's forward vector.
	ServerThrowObject(ObjectCarryPoint->GetForwardVector());
}

void ABKCharacter::ServerThrowObject_Implementation(const FVector ClientForwardVector)
{
	if (GrabbedObject != nullptr && GrabbedObject->IsA<ABKBall>())
	{
		const FVector ShootVelocity = ClientForwardVector * ShootStrength;

		//RootComponent of the ABKBall class is it's static mesh.
		UStaticMeshComponent* ObjectMesh = Cast<UStaticMeshComponent>(GrabbedObject->GetRootComponent());
		if (ObjectMesh != nullptr)
		{
			//ObjectMesh->SetSimulatePhysics(false);
			GrabbedObject->SetActorEnableCollision(true);
			ObjectMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			ObjectMesh->SetSimulatePhysics(true);
			ObjectMesh->AddImpulse(ShootVelocity, NAME_None, true);
		}

		//Up-casting from AActor to ABKBall.
		ABKBall* ball = Cast<ABKBall>(GrabbedObject);

		//Setting IsGrabbed false since the player throw the ball.
		if (ball != nullptr)
			ball->IsGrabbed = false;

		//Setting GrabbedObject to nullptr as we are no longer carrying it.
		GrabbedObject = nullptr;
		OnBallDrop();
		IsCarryingBall = false;
	}
}