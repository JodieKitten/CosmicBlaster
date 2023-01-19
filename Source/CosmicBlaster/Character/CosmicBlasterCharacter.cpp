// Fill out your copyright notice in the Description page of Project Settings.


#include "CosmicBlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"

ACosmicBlasterCharacter::ACosmicBlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->TargetArmLength = 600.f;
	SpringArm->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);
}

void ACosmicBlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ACosmicBlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACosmicBlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACosmicBlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ACosmicBlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ACosmicBlasterCharacter::LookUp);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
}

void ACosmicBlasterCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ACosmicBlasterCharacter::MoveRight(float Value)
{
	AddControllerYawInput(Value);
}

void ACosmicBlasterCharacter::Turn(float Value)
{
}

void ACosmicBlasterCharacter::LookUp(float Vlaue)
{
}

void ACosmicBlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



