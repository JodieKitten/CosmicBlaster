// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CosmicBlaster/CosmicBlaster.h"
#include "BlasterGameMode.generated.h"

class ACosmicBlasterCharacter;
class ABlasterPlayerController;
class ABlasterPlayerState;



namespace MatchState
{
	extern COSMICBLASTER_API const FName Cooldown; // match ended, display winner and start cooldown timer
}

UCLASS()
class COSMICBLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ABlasterGameMode();
	virtual void Tick(float Deltatime) override;
	virtual void PlayerEliminated(ACosmicBlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	void PlayerLeftGame(ABlasterPlayerState* PlayerLeaving);
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

	bool bTeamsMatch = false;



protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;

	UPROPERTY(EditDefaultsOnly)
	float NearbyRadius = 500.0f;

	UPROPERTY(EditDefaultsOnly)
	FVector DefaultLocation = FVector(0, 0, 0);

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
