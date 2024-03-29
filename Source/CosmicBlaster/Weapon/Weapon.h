// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "CosmicBlaster/BlasterTypes/Team.h"
#include "CosmicBlaster/Interfaces/InteractInterface.h"
#include "Weapon.generated.h"

class UBoxComponent;
class UWidgetComponent;
class UAnimationAsset;
class UTexture2D;
class ACosmicBlasterCharacter;
class ABlasterPlayerController;
class USoundCue;
class UPickupWidget;
class USpotLightComponent;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class COSMICBLASTER_API AWeapon : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	virtual void InteractableFound_Implementation(ACosmicBlasterCharacter* OverlappingPlayer) override;

	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;

	UPROPERTY(EditAnywhere)
	EFireType FireType;

	virtual void Fire(const FVector& HitTarget);
	virtual void Dropped();
	void SetHUDAmmo();
	void AddAmmo(int32 AmmoToAdd);

	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;

	FVector TraceEndWithScatter(const FVector& HitTarget);

	/* HUD */
	void ShowPickupWidget(bool bShowWidget, bool bUsingController);

	/* Textures for crosshairs */
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsCentre;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;

	/* Equip */
	UPROPERTY(EditAnywhere)
	USoundCue* EquipSound;

	/* Automatic Fire */
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.15f;

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;

	bool IsEmpty();
	bool IsFull();

	/* Enable / disable custom depth - weapon outline colour */

	void EnableCustomDepth(bool bEnable);

	void SetSpotLight(bool bIsOn);

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USpotLightComponent* SpotLight;

protected:
	virtual void BeginPlay() override;
	void PollInit();

	bool bHasSetController = false;

	UPROPERTY()
	ACosmicBlasterCharacter* BlasterOwnerCharacter;

	UPROPERTY()
	ABlasterPlayerController* BlasterOwnerController;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnEquippedSecondary();
	virtual void OnDropped();

	/* Trace end with scatter */
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

	UPROPERTY(EditAnywhere)
	float SSRDamage = 8.f;

	UPROPERTY(EditAnywhere)
	float SSRHeadShotDamage = 10.f;

	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;

	UPROPERTY(EditAnywhere)
	bool bUseServerSideRewindDefault = false;

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);
private:
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	/*UPROPERTY(EditAnywhere)
	ETeam Team;*/

	/* Components */
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UBoxComponent* BoxCollision;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UWidgetComponent* PickupWidget;

	UPROPERTY(ReplicatedUsing = OnRep_SetSpotLight)
	bool bUsingSpotLight = false;

	UFUNCTION()
	void OnRep_SetSpotLight();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	/* Zoom whilst aiming */
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float ZoomInterpSpeed = 20.f;

	/* States */
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState,VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	/* Animations */
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UAnimationAsset* FireAnimation;

	/* Ammo */
	UPROPERTY(EditAnywhere)
	int32 Ammo;

	void SpendRound();

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastAddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	// no. of unprocessed server requests for ammo
	//incremeneted in spend round, decremented in updateammo
	int32 Sequence = 0;

public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE UBoxComponent* GetBoxCollision() const { return BoxCollision; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
	FORCEINLINE float GetSSRDamage() const { return SSRDamage; }
	FORCEINLINE float GetSSRHeadShotDamage() const { return SSRHeadShotDamage; }
	//FORCEINLINE ETeam GetTeam() const { return Team; }
};
