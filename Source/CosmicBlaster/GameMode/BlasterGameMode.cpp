// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "CosmicBlaster/Character/CosmicBlasterCharacter.h"
#include "CosmicBlaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "CosmicBlaster/PlayerState/BlasterPlayerState.h"
#include "CosmicBlaster/GameState/BlasterGameState.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::Tick(float Deltatime)
{
	Super::Tick(Deltatime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame(); //unreal engine function
		}
	}
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) //loop through all player controllers 
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It); // dereferecing will give the player controller
		if (BlasterPlayer)
		{
			BlasterPlayer->OnMatchStateSet(MatchState, bTeamsMatch); //sets all player controllers match state
		}
	}
}

float ABlasterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}

void ABlasterGameMode::PlayerEliminated(ACosmicBlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState)
	{
		TArray<ABlasterPlayerState*> PlayersCurrentlyInTheLead;
		for (auto LeadPlayer : BlasterGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}

		AttackerPlayerState->AddToScore(1.f);
		BlasterGameState->UpdateTopScore(AttackerPlayerState);
		if (BlasterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			ACosmicBlasterCharacter* Leader = Cast<ACosmicBlasterCharacter>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!BlasterGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				ACosmicBlasterCharacter* Loser = Cast<ACosmicBlasterCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(AttackerController, false);
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if (BlasterPlayer && AttackerPlayerState && VictimPlayerState)
		{
			BlasterPlayer->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray <AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);

		//have to cast to the character of the elimmed controller - not the elimmed character itself
		//ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(ElimmedController);
		ACosmicBlasterCharacter* BlasterCharacter = Cast<ACosmicBlasterCharacter>(ElimmedController->GetPawn());

		if (MatchState == MatchState::Cooldown && BlasterCharacter)
		{
			BlasterCharacter->bDisableGameplay = true;

			/*BlasterCharacter->PlayMacerenaMontage();
			PlayerController->CooldownCelebration();
			PlayerController->FireworkCelebration();

			PlayerController->CooldownFunctions();*/
		}
	}



	/*// Find all available player starts
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

	// Define maximum number of attempts to find a suitable spawn location
	const int32 MaxAttempts = 10;

	// Loop through all player starts for a fixed number of attempts and select the first one that has no overlapping actors
	for (int32 i = 0; i < MaxAttempts; i++)
	{
		// Select a random player start
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		FVector StartLocation = PlayerStarts[Selection]->GetActorLocation();

		// Check for nearby players
		TArray<FOverlapResult> Overlaps;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(ElimmedCharacter);
		QueryParams.bTraceComplex = true;
		GetWorld()->OverlapMultiByObjectType(
			Overlaps,
			StartLocation,
			FQuat::Identity,
			FCollisionObjectQueryParams(ECC_SkeletalMesh),
			FCollisionShape::MakeSphere(NearbyRadius),
			QueryParams);

		UE_LOG(LogTemp, Warning, TEXT("Number of overlapping actors found: %d"), Overlaps.Num());

		// If no nearby players are found, respawn at the current player start and exit the loop
		if (Overlaps.Num() == 0)
		{
			RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
			UE_LOG(LogTemp, Warning, TEXT("Respawn Successful for: %s"), *ElimmedController->GetName());
			return;
		}
	}
	// If no suitable spawn location is found, use a default location or return an error
	UE_LOG(LogTemp, Warning, TEXT("No suitable spawn location found for player %s"), *ElimmedController->GetName());
	FRotator DefaultRotation = FRotator(0.0f, 0.0f, 0.0f);
	RestartPlayerAtTransform(ElimmedController, FTransform(DefaultRotation, DefaultLocation));

	if (MatchState == MatchState::Cooldown)
	{
		ACosmicBlasterCharacter* Character = Cast<ACosmicBlasterCharacter>(ElimmedCharacter);
		Character->bDisableGameplay = true;
	}*/
}

void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	ACosmicBlasterCharacter* CharacterLeaving = Cast<ACosmicBlasterCharacter>(PlayerLeaving->GetPawn());
	ABlasterPlayerController* CharacterController = Cast<ABlasterPlayerController>(PlayerLeaving->GetPlayerController());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(CharacterController, true);
	}
}
