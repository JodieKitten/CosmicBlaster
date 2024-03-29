// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;
class UHorizontalBox;
class UBorder;

UCLASS()
class COSMICBLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BlueTeamScore;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BlueTeamText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RedTeamScore;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RedTeamText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponType;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ElimText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadesText;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* Blink;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;

	UPROPERTY(meta = (BindWidget))
	UImage* HighPingImage;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* HighPingAnimation;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GameOverText;

	UPROPERTY(meta = (BindWidget))
	UImage* BlueFlagImage;

	UPROPERTY(meta = (BindWidget))
	UImage* RedFlagImage;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* PickedUpAmmoBox;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* PickedUpAmmoText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* AmmoAmountText;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* HitMarkerAnimation;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HitMarker;

	UPROPERTY(meta = (BindWidget))
	UBorder* AmmoBorder;
};
