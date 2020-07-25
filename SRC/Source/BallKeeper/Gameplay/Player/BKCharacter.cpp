// Copyright Emre Bugday (emreb25@outlook.com). All Rights Reserved.

#include "BKCharacter.h"

#include "DrawDebugHelpers.h"
#include "BallKeeper/Framework/BKGameModeBase.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

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

void ABKCharacter::SpawnPlayer_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Spawning player..."));
	
	ABKGameModeBase* GM = Cast<ABKGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	
	if (TeamId == 1)
		SetActorLocation(GM->TeamOneSpawnPoint);
	else if (TeamId == 2)
		SetActorLocation(GM->TeamTwoSpawnPoint);
}

void ABKCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicates to everyone
	DOREPLIFETIME(ABKCharacter, TeamId);
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
		AddMovementInput(GetActorRightVector(), value);
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

	APlayerController* PC = Cast<APlayerController>(this->GetController());

	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector Start = CameraLocation;
	const FVector ForwardVector = CameraLocation.ForwardVector;
	const FVector End = CameraLocation + (CameraRotation.Vector() * ReachDistance);
	FHitResult Hit;

	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams))
		return;

	UPrimitiveComponent* PrimComp = Hit.GetComponent();
	GrabbedObject = Hit.GetActor();

	if (GrabbedObject->IsA<ABKBall>())
	{
		//if (!GrabbedObject)
		//	return;
		//	
		
		ABKBall* ball = Cast<ABKBall>(GrabbedObject);
		if (ball != nullptr)
		{
			ball->IsGrabbed = true;
			ball->LastTeamId = TeamId;
		}

		UStaticMeshComponent* ObjectMesh = Cast<UStaticMeshComponent>(GrabbedObject->GetRootComponent());
		if (ObjectMesh != nullptr)
			ObjectMesh->SetSimulatePhysics(false);

		GrabbedObject->AttachToComponent(this->ObjectCarryPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

void ABKCharacter::StopFire()
{
	ServerThrowObject(CameraComponent->GetForwardVector());
}

void ABKCharacter::ServerThrowObject_Implementation(FVector ClientForwardVector)
{
	if (GrabbedObject)
	{
		const FVector ShootVelocity = ClientForwardVector * ShootStrength;

		UStaticMeshComponent* ObjectMesh = Cast<UStaticMeshComponent>(GrabbedObject->GetRootComponent());
		if (ObjectMesh != nullptr)
		{
			ObjectMesh->SetSimulatePhysics(false);
			ObjectMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			ObjectMesh->SetSimulatePhysics(true);
			ObjectMesh->AddImpulse(ShootVelocity, NAME_None, true);
		}

		//Up-casting from AActor to ABKBall
		ABKBall* ball = Cast<ABKBall>(GrabbedObject);
		if (ball != nullptr)
		{
			ball->IsGrabbed = false;
		}

		GrabbedObject = nullptr;
	}
}