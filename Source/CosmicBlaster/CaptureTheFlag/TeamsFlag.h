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

	void FlagBehavior();

	void EnableCustomDepth(bool bEnable);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFlagRespawn();

	void SetFlagState(EFlagState State);

	void OnDropped();

	void OnEquipped();

	void OnInitial();

	void DetachfromBackpack();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDetachfromBackpack();

	UPROPERTY(EditAnywhere)
	float EquippedFlagSpeed = 600.f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,	int32 OtherBodyIndex, bool bFromSweep,	const FHitResult& SweepResult);

	UPROPERTY(BlueprintReadOnly, Category = "Flag")
	ACosmicBlasterCharacter* OwningCharacter;

	UPROPERTY()
	ABlasterPlayerController* OwningController;

	UPROPERTY(VisibleAnywhere, Category = "Flag Properties")
	USphereComponent* OverlapSphere;

private:
	UPROPERTY(ReplicatedUsing = OnRep_FlagState, VisibleAnywhere)
	EFlagState FlagState;

	UFUNCTION()
	void OnRep_FlagState();

	UPROPERTY(VisibleAnywhere, Category = "Flag Properties")
	UStaticMeshComponent* FlagMesh;

	UPROPERTY(EditAnywhere)
	EFlagType FlagType;

	UPROPERTY(EditAnywhere)
	ETeam Team;

	UPROPERTY(EditAnywhere)
	FTransform InitialSpawnLocation;

	FTimerHandle BindOverlapTimer;
	float BindOverlapTime = 0.25f;
//	void BindOverlapTimerFinished();

public:
	FORCEINLINE EFlagState GetFlagState() const { return FlagState; }
	FORCEINLINE void SetFlagStateOD(EFlagState State) { FlagState = State; }
	FORCEINLINE UStaticMeshComponent* GetFlagMesh() const { return FlagMesh; }
	FORCEINLINE EFlagType GetFlagType() const { return FlagType; }
	FORCEINLINE ETeam GetTeam() const { return Team; }
};