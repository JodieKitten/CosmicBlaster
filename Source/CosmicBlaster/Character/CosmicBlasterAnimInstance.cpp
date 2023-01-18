// Fill out your copyright notice in the Description page of Project Settings.


#include "CosmicBlasterAnimInstance.h"
#include "CosmicBlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UCosmicBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ACosmicBlasterCharacter>(TryGetPawnOwner());
}

void UCosmicBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ACosmicBlasterCharacter>(TryGetPawnOwner());
	}

	if (BlasterCharacter == nullptr) return;

	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
}
