// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CosmicBlaster/Weapon/WeaponTypes.h"
#include "EnhancedInput/Public/InputAction.h"
#include "CosmicBlaster/BlasterTypes/InputTypes.h"
#include "CosmicBlaster/CaptureTheFlag/TeamsFlag.h"
#include "BlasterPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

class ABlasterHUD;
class UCharacterOverlay;
class ABlasterGameMode;
class ABlasterPlayerState;
class UUserWidget;
class UReturnToMainMenu;
class UCombatComponent;
class ABlasterPlayerState;
class ABlasterGameState;
class ACosmicBlasterCharacter;
class UTexture2D;

UCLASS()
class COSMICBLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	FHighPingDelegate HighPingDelegate;

	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void Interact();

	UFUNCTION(Server, Reliable)
	void ServerInteract();

	/* Controller / Keyboard */
	void DetermineInputDeviceDetails(FKey KeyPressed);
	bool bIsInputDeviceGamepad = false;

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDWeaponType(EWeaponType WeaponType);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetElimText(FString InText);
	void ClearElimText();
	void HideTeamScores();
	void InitTeamScores();
	void SetHUDRedTeamScore(int32 RedScore);
	void SetHUDBlueTeamScore(int32 BlueScore);
	void SetHUDGrenades(int32 Grenades);
	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);
	void UpdateBlueFlagStateInHUD(EFlagState NewFlagState);
	void UpdateRedFlagStateInHUD(EFlagState NewFlagState);

	virtual float GetServerTime(); //synced with server world clock
	virtual void ReceivedPlayer() override; // sync with server clock asap
	float SingleTripTime = 0.f;

	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	void HandleMatchHasStarted(bool bTeamsMatch = false);
	void HandleCooldown();
	void CooldownCelebration();
	void FireworkCelebration();
	void CooldownFunctions();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	void SetHUDTime();
	void PollInit();
	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);

	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores();

	FString GetInfoText(const TArray<ABlasterPlayerState*>& Players);
	FString GetTeamsInfoText(ABlasterGameState* BlasterGameState);

	void ShowReturnToMainMenu();

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	/* sync time between client and server */

	// requests current server time, passing in client's time of request
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// reports current server time to client in response to request
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	//difference between client and server time
	float ClientServerDelta = 0.f;

	//syncs up time every x seconds
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

private:
	UPROPERTY()
	ABlasterHUD* BlasterHUD;

	UPROPERTY()
	ABlasterGameMode* BlasterGameMode;

	/* return to main menu */
	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<UUserWidget> ReturnToMainMenuWidget;

	UPROPERTY()
	UReturnToMainMenu* ReturnToMainMenu;

	bool bReturnToMainMenuOpen = false;

	float LevelStartingTime;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;
	bool bInitializeHealth = false;
	bool bInitializeScore = false;
	bool bInitializeDefeats = false;
	bool bInitializeGrenades = false;
	bool bInitializeShield = false;
	bool bInitializeCarriedAmmo = false;
	bool bInitializeWeaponAmmo = false;
	bool bInitializeWeaponType = false;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDShield;
	float HUDMaxShield;
	float HUDScore;
	int32 HUDDefeats;
	int32 HUDGrenades;
	float HUDCarriedAmmo;
	float HUDWeaponAmmo;
	EWeaponType HUDWeaponType;

	float HighPingRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 100.f;

	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	UTexture2D* BlueFlagInitial;

	UPROPERTY(EditAnywhere)
	UTexture2D* BlueFlagStolen;

	UPROPERTY(EditAnywhere)
	UTexture2D* BlueFlagDropped;

	UPROPERTY(EditAnywhere)
	UTexture2D* RedFlagInitial;

	UPROPERTY(EditAnywhere)
	UTexture2D* RedFlagStolen;

	UPROPERTY(EditAnywhere)
	UTexture2D* RedFlagDropped;
};
