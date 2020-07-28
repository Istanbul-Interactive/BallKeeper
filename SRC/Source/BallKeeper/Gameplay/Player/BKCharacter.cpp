// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#include "BKCharacter.h"

#include "BKPlayerControllerBase.h"
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

	SpotLight = CreateDefaultSubobject<USpotLightComponent>("SpotLightComponent");
	SpotLight->SetupAttachment(CameraComponent, HeadSocketName);

	ObjectCarryPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ObjectCarryPoint"));
	ObjectCarryPoint->SetupAttachment(CameraComponent);

	ReachDistance = 1000.0f;
	DashPower = 3000.0f;
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
	PlayerInputComponent->BindAction("DashRight", IE_Pressed, this, &ABKCharacter::ClientDashRight);
	PlayerInputComponent->BindAction("DashLeft", IE_Pressed, this, &ABKCharacter::ClientDashLeft);
	PlayerInputComponent->BindAction("DashForward", IE_Pressed, this, &ABKCharacter::ClientDashForward);
	PlayerInputComponent->BindAction("DashBackward", IE_Pressed, this, &ABKCharacter::ClientDashBackward);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABKCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABKCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ABKCharacter::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &ABKCharacter::Turn);
}

void ABKCharacter::ResetPlayerPosition_Implementation()
{
	ABKGameModeBase* GM = Cast<ABKGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	ABKPlayerControllerBase* PC = Cast<ABKPlayerControllerBase>(GetController());

	if (PC == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Player Controller NULLPTR!!!"));
		return;
	}

	const FVector RandomVectorToAdd = FVector(FMath::RandRange(100.0f, 400.0f), FMath::RandRange(100.0f, 400.0f), FMath::RandRange(100.0f, 400.0f));

	//Spawning player according to their team spawn point.
	if (PC->TeamId == 1)
	{
		SetActorLocation(GM->TeamOneSpawnPointLocation + RandomVectorToAdd);
		SetActorRotation(GM->TeamOneSpawnPointRotation);
	}
	else if (PC->TeamId == 2)
	{
		SetActorLocation(GM->TeamTwoSpawnPointLocation + RandomVectorToAdd);
		SetActorRotation(GM->TeamTwoSpawnPointRotation);
	}
}

void ABKCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicates to everyone
	DOREPLIFETIME(ABKCharacter, IsCarryingBall);
	DOREPLIFETIME(ABKCharacter, TeamId);
}

APlayerController* ABKCharacter::GetPlayerController() const
{
	return Cast<APlayerController>(Controller);
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

void ABKCharacter::ClientDashForward()
{
	ServerDashForward();
}

void ABKCharacter::ClientDashBackward()
{
	ServerDashBackward();
}

void ABKCharacter::ClientDashRight()
{
	ServerDashRight();
}

void ABKCharacter::ClientDashLeft()
{
	ServerDashLeft();
}

void ABKCharacter::ServerDashBackward_Implementation()
{
	FVector LaunchVector = GetActorForwardVector() * -DashPower;

	LaunchCharacter(LaunchVector, false, false);
}

void ABKCharacter::ServerDashForward_Implementation()
{
	FVector LaunchVector = GetActorForwardVector() * DashPower;

	LaunchCharacter(LaunchVector, false, false);
}

void ABKCharacter::ServerDashRight_Implementation()
{
	FVector LaunchVector = GetActorRightVector() * DashPower;

	LaunchCharacter(LaunchVector, false, false);
}

void ABKCharacter::ServerDashLeft_Implementation()
{
	FVector LaunchVector = GetActorRightVector() * -DashPower;

	LaunchCharacter(LaunchVector, false, false);
}

void ABKCharacter::StartFire()
{
	ServerGrabObject();
}

void ABKCharacter::ServerGrabObject_Implementation()
{
	const FCollisionQueryParams QueryParams("ObjectCarryTrace", false, this);

	//Casting the controller to APlayerController class.
	ABKPlayerControllerBase* PC = Cast<ABKPlayerControllerBase>(GetController());

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
			ball->LastTeamId = PC->TeamId;
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
		IsCarryingBall = false;
		OnBallDrop();
	}
}