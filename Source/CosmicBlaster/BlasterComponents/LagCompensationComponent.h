// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class ABlasterPlayerController;
class ACosmicBlasterCharacter;
class AWeapon;

USTRUCT(BlueprintType)
struct FBoxInformation //information for the boxes themselves on the character
{
	GENERATED_BODY()

		UPROPERTY()
		FVector Location;

	UPROPERTY()
		FRotator Rotation;

	UPROPERTY()
		FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage //packages full of the information for the boxes on character in the struct above
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COSMICBLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	friend class ACosmicBlasterCharacter;
	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);
	FServerSideRewindResult ServerSideRewind(ACosmicBlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);
	
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(ACosmicBlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser);

protected:
	virtual void BeginPlay() override;
	void SaveFramePackage();
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, ACosmicBlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);
	void CacheBoxPosition(ACosmicBlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(ACosmicBlasterCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(ACosmicBlasterCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(ACosmicBlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

private:
	UPROPERTY()
	ACosmicBlasterCharacter* Character;

	UPROPERTY()
	ABlasterPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
};
