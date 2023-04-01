// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlagTypes.h"
#include "CosmicBlaster/BlasterTypes/Team.h"
#include "TeamsFlag.generated.h"

class ACosmicBlasterCharacter;
class ABlasterPlayerController;
class USphereComponent;

UENUM(BlueprintType)
enum class EFlagState : uint8
{
	EFS_Initial UMETA(DisplayName = "Initial State"),
	EFS_Equipped UMETA(DisplayName = "Equipped"),
	EFS_Dropped UMETA(DisplayName = "Dropped"),

	EFS_MAX UMETA(DisplayName = "DefaultMAX"),

};

UCLASS()
class COSMICBLASTER_API ATeamsFlag : public AActor
{
	GENERATED_BODY()

public:
	ATeamsFlag();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EnableCustomDepth(bool bEnable);

	void SetFlagState(EFlagState State);

	void OnFlagStateSet();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDropped();

	void RespawnFlag();

	UPROPERTY(EditAnywhere)
	float EquippedFlagSpeed = 600.f;

//	void UpdateFlagHUD();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,	int32 OtherBodyIndex, bool bFromSweep,	const FHitResult& SweepResult);

	UPROPERTY(BlueprintReadOnly, Category = "Flag")
	ACosmicBlasterCharacter* OwningCharacter;

	UPROPERTY()
	ABlasterPlayerController* OwningController;

	UPROPERTY(VisibleAnywhere, Category = "Flag Properties")
	USphereComponent* Sphere;

private:
	UPROPERTY(ReplicatedUsing = OnRep_FlagState, VisibleAnywhere)
	EFlagState FlagState;

	UFUNCTION()
	void OnRep_FlagState();

	UPROPERTY(VisibleAnywhere, Category = "Flag Properties")
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere)
	EFlagType FlagType;

	UPROPERTY(EditAnywhere)
	ETeam Team;

	UPROPERTY(EditAnywhere)
	FVector InitialSpawnLocation;

	UPROPERTY(EditAnywhere)
	FTransform InitialSpawnTransform;

public:
	FORCEINLINE FTransform GetInitialTransform() const { return InitialSpawnTransform; }
	FORCEINLINE EFlagState GetFlagState() const { return FlagState; }
	FORCEINLINE void SetFlagStateOD(EFlagState State) { FlagState = State; }
	FORCEINLINE UStaticMeshComponent* GetFlagMesh() const { return Mesh; }
	FORCEINLINE EFlagType GetFlagType() const { return FlagType; }
	FORCEINLINE ETeam GetTeam() const { return Team; }
};