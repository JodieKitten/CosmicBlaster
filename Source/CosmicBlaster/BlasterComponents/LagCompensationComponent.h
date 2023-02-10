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

	UPROPERTY()
	ACosmicBlasterCharacter* Character;
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

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<ACosmicBlasterCharacter*, uint32> HeadShots;
	UPROPERTY()
	TMap<ACosmicBlasterCharacter*, uint32> BodyShots;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COSMICBLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	friend class ACosmicBlasterCharacter;

	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	FServerSideRewindResult ServerSideRewind(ACosmicBlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);
	FServerSideRewindResult ProjectileServerSideRewind(ACosmicBlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<ACosmicBlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(ACosmicBlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(const TArray<ACosmicBlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(ACosmicBlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);

protected:
	virtual void BeginPlay() override;

	void SaveFramePackage();
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage GetFrameToCheck(ACosmicBlasterCharacter* HitCharacter, float HitTime);

	void EnableCharacterMeshCollision(ACosmicBlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

	void CacheBoxPosition(ACosmicBlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(ACosmicBlasterCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(ACosmicBlasterCharacter* HitCharacter, const FFramePackage& Package);

	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, ACosmicBlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);
	FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& Package, ACosmicBlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
	FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);

private:
	UPROPERTY()
	ACosmicBlasterCharacter* Character;

	UPROPERTY()
	ABlasterPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
};
