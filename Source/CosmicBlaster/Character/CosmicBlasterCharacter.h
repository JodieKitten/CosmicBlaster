// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CosmicBlaster/BlasterTypes/TurningInPlace.h"
#include "CosmicBlasterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;

UCLASS()
class COSMICBLASTER_API ACosmicBlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACosmicBlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;

	/* Replication */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	 /* Inputs */
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();

	void AimOffset(float DeltaTime);

private:
	/* Replication */
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed(); // RPC's (remote procedure calls) functions for client to server (otherwise other way around)

	/* Components */
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere)
	UCombatComponent* Combat;

	/* HUD */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	/* Weapon */
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon) //will call the function when overlapping the weapon
	AWeapon* OverlappingWeapon;

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;

	void TurnInPlace(float DeltaTime);

public:
	//getters and setters
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	AWeapon* GetEquippedWeapon();
	
};
