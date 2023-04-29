// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CosmicBlaster/Weapon/WeaponTypes.h"
#include "PickupAmmo.generated.h"

/**
 * 
 */
UCLASS()
class COSMICBLASTER_API UPickupAmmo : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void SetPickedUpAmmoText(EWeaponType WeaponType, int32 AmmoAmount);

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* PickedUpAmmoBox;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PickedUpAmmoText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AmmoAmountText;
};
