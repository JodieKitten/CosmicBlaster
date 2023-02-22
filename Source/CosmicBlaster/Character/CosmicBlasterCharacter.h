// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CosmicBlaster/BlasterTypes/TurningInPlace.h"
#include "CosmicBlaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "CosmicBlaster/BlasterTypes/CombatState.h"
#include "CosmicBlaster/BlasterTypes/Team.h"
#include "CosmicBlasterCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;
class UAnimMontage;
class ABlasterPlayerController;
class USoundCue;
class ABlasterPlayerState;
class UBuffComponent;
class UBoxComponent;
class ULagCompensationComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class ABlasterGameMode;

UCLASS()
class COSMICBLASTER_API ACosmicBlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ACosmicBlasterCharacter();
	void SpawnDefaultWeapon();
	bool bFinishedSwapping = false;

	/* hit boxes used for server side rewind */
	UPROPERTY(EditAnywhere)
	UBoxComponent* Head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArm_L;

	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArm_R;

	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerArm_L;

	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerArm_R;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Hand_L;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Hand_R;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Backpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Blanket;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Thigh_L;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Thigh_R;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Calf_L;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Calf_R;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Foot_L;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Foot_R;

	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxes;

	UPROPERTY()
	ABlasterPlayerState* BlasterPlayerState;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);
	
	/* Overrides */
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	virtual void OnRep_ReplicatedMovement() override;

	/* HUD */
	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();

	/*Montages*/
	void PlayFireMontage(bool bAiming);
	void PlayElimMontage();
	void PlayReloadMontage();
	void PlayThrowGrenadeMontage();
	void PlaySwapMontage();
	void PlayMacerenaMontage();

	/* Replication */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/* Teams */
	void SetTeamColour(ETeam Team);

	/* Elimination */
	virtual void Destroyed() override;

	void Elim(APlayerController* AttackerController, bool bPlayerLeftGame);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(const FString& AttackerName, bool bPlayerLeftGame);

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	FOnLeftGame OnLeftGame;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	UPROPERTY(EditAnywhere)
	UAnimMontage* MacerenaMontage;


	//// macerena test
	UPROPERTY(ReplicatedUsing = OnRep_PlayMacerena)
	bool bShouldPlayMacerena = false;

	bool bCooldown = false;

	UFUNCTION()
	void OnRep_PlayMacerena();

	UFUNCTION(Server, Reliable)
	void ServerPlayMacerena(bool bPlayMacerena);

	void PlayMacerena(bool bPlayMacerena);

	void PlayFireworks();

protected:
	virtual void BeginPlay() override;
	void PollInit(); // poll for any relevant classes and initialize the HUD
	void DropOrDestroyWeapon(AWeapon* Weapon);
	void DropOrDestroyWeapons();
	void SetSpawnPoint();
	void OnPlayerStateInitialized();

	 /* Inputs */
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();
	virtual void Jump() override;
	void ReloadButtonPressed();
	void GrenadeButtonPressed();

	/* Aim Offset */
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();
	void RotateInPlace(float DeltaTime);

	/* Montages */
	void PlayHitReactMontage();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

private:
	UPROPERTY()
	ABlasterGameMode* BlasterGameMode;

	/* Movement */
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);
	float CalculateSpeed();
	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;

	/* Replication */
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed(); // RPC's (remote procedure calls) functions for client to server (otherwise other way around)

	/* Components */
	void HideCharacterIfCameraClose();

	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UBuffComponent* Buff;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	ULagCompensationComponent* LagCompensation;

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	/* HUD */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	/* Weapon */
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon) //will call the function when overlapping the weapon
	AWeapon* OverlappingWeapon;

	/* Montages */
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapMontage;

	/* Aim Offset */
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	/* Health */
	UPROPERTY()
	ABlasterPlayerController* BlasterPlayerController;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/* Shield */
	UPROPERTY(VisibleAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
	float Shield = 50.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	/* Team Colours */

	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* RedDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Teams)
	UMaterialInstance* RedMaterial;

	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* BlueDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Teams)
	UMaterialInstance* BlueMaterial;

	UPROPERTY(EditAnywhere, Category = Teams)
	UMaterialInstance* OriginalMaterial;

	/* Elimination */
	bool bElimmed = false;
	FTimerHandle ElimTimer;
	void ElimTimerFinished();

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	bool bLeftGame = false;

	/* Dissolve effect */
	FOnTimelineFloat DissolveTrack;
	void StartDissolve();

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UPROPERTY(VisibleAnywhere, Category = Elimination)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance; //instance to change at run time

	UPROPERTY(VisibleAnywhere, Category = Elimination)
	UMaterialInstance* DissolveMaterialInstance; //instance set on BP, used with dynamic mat instance

	/* Elimination Bot */
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* ElimBotSound;

	/* Winning / Crown */
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* CrownSystem;

	UPROPERTY()
	UNiagaraComponent* CrownComponent;

	/* Grenade */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	/* Default Weapon */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	bool bInitializeStartingWeapon = false;

	AWeapon* StartingWeapon;


	UPROPERTY(EditAnywhere)
	class UParticleSystem* FireworkSystem;

	UPROPERTY()
	class UParticleSystemComponent* FireworkComponent;

public:
	//getters and setters
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	AWeapon* GetEquippedWeapon();
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
	bool IsLocallyReloading();
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }
	bool IsHoldingTheFlag() const;
	ETeam GetTeam();
	void SetHoldingTheFlag(bool bHolding);
	bool ShouldPlayMacerenaMontage();
};
