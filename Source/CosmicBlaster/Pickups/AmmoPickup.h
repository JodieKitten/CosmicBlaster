// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "CosmicBlaster/Weapon/WeaponTypes.h"
#include "AmmoPickup.generated.h"

/**
 * 
 */
UCLASS()
class COSMICBLASTER_API AAmmoPickup : public APickup
{
	GENERATED_BODY()
	
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere)
	int32 AmmoAmount = 30.f;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

public:
	FORCEINLINE int32 GetAmmoAmount() { return AmmoAmount; }
	FORCEINLINE EWeaponType GetWeaponType() { return WeaponType; }
};
