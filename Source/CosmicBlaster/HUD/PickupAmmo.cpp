// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupAmmo.h"
#include "Components/TextBlock.h"

void UPickupAmmo::SetPickedUpAmmoText(EWeaponType WeaponType, int32 AmmoAmount)
{
	FString TypeOfWeapon;
	switch (WeaponType)
	{
	case EWeaponType::EWT_AssaultRifle:
		TypeOfWeapon = FString::Printf(TEXT("Assault Rifle"));
		break;
	case EWeaponType::EWT_GrenadeLauncher:
		TypeOfWeapon = FString::Printf(TEXT("Grenade Launcher"));
		break;
	case EWeaponType::EWT_Pistol:
		TypeOfWeapon = FString::Printf(TEXT("Pistol"));
		break;
	case EWeaponType::EWT_RocketLauncher:
		TypeOfWeapon = FString::Printf(TEXT("Rocket Launcher"));
		break;
	case EWeaponType::EWT_Shotgun:
		TypeOfWeapon = FString::Printf(TEXT("Shotgun"));
		break;
	case EWeaponType::EWT_SniperRifle:
		TypeOfWeapon = FString::Printf(TEXT("Sniper"));
		break;
	case EWeaponType::EWT_SubmachineGun:
		TypeOfWeapon = FString::Printf(TEXT("Submachine Gun"));
		break;
	}

	if (PickedUpAmmoText)
	{
		PickedUpAmmoText->SetText(FText::FromString(TypeOfWeapon));
		AmmoAmountText->SetText(FText::AsNumber(AmmoAmount));
	}
}
