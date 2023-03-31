// Fill out your copyright notice in the Description page of Project Settings.


#include "CosmicBlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "CosmicBlaster/Weapon/Weapon.h"
#include "CosmicBlaster/BlasterComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "CosmicBlasterAnimInstance.h"
#include "CosmicBlaster/CosmicBlaster.h"
#include "CosmicBlaster/PlayerController/BlasterPlayerController.h"
#include "CosmicBlaster/GameMode/BlasterGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "CosmicBlaster/PlayerState/BlasterPlayerState.h"
#include "CosmicBlaster/Weapon/WeaponTypes.h"
#include "CosmicBlaster/BlasterComponents/BuffComponent.h"
#include "Components/BoxComponent.h"
#include "CosmicBlaster/BlasterComponents/LagCompensationComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "CosmicBlaster/GameState/BlasterGameState.h"
#include "CosmicBlaster/Weapon/Flag.h"
#include "CosmicBlaster/PlayerStart/TeamPlayerStart.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "CosmicBlaster/Interfaces/InteractInterface.h"
#include "CosmicBlaster/HUD/BlasterHUD.h"
#include "CosmicBlaster/HUD/CharacterOverlay.h"
#include "CosmicBlaster/CaptureTheFlag/TeamsFlag.h"

#include "CosmicBlaster/HUD/OverheadWidget.h"
#include "Components/SpotLightComponent.h"

/*
Initial functions
*/


void ACosmicBlasterCharacter::ScanForInteractables()
{
	FHitResult HitResult;
	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = Start + (FollowCamera->GetComponentRotation().Vector() * 500.F);

	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjects;
	TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> FoundCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACosmicBlasterCharacter::StaticClass(), FoundCharacters);

	for (AActor* Actor : FoundCharacters)
	{
		ACosmicBlasterCharacter* BlasterCharacter = Cast<ACosmicBlasterCharacter>(Actor);
		ActorsToIgnore.Add(BlasterCharacter);
	}
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(Combat->EquippedWeapon);
	ActorsToIgnore.Add(Combat->SecondaryWeapon);

	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

	const bool bIsInteractable = UKismetSystemLibrary::LineTraceSingleForObjects(
		GetWorld(),
		Start,
		End,
		TraceObjects,
		true,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		HitResult,
		true
	);
	
	if (bIsInteractable)
	{			
		if (HitResult.GetActor())
		{
			if (HitResult.GetActor()->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
			{
				IInteractInterface::Execute_InteractableFound(HitResult.GetActor(), this);
				//GEngine->AddOnScreenDebugMessage(-1, 8.F, FColor::FromHex("#FFD801"), __FUNCTION__);
			}
			else
			{
				SetOverlappingWeapon(nullptr);
			}
		}	
	}
	else
	{
		SetOverlappingWeapon(nullptr);
	}
}

void ACosmicBlasterCharacter::ShowOverheadWidget(ACosmicBlasterCharacter* FoundCharacter, bool bShowOverheadWidget)
{
	if (OverheadWidget)
	{
		OverheadWidget->SetVisibility(bShowOverheadWidget);

		if (FoundCharacter)
		{
			UOverheadWidget* NameWidget = Cast<UOverheadWidget>(OverheadWidget->GetUserWidgetObject());
			if (NameWidget)
			{
				NameWidget->ShowPlayerName(FoundCharacter);
			}
		}
	}
}

void ACosmicBlasterCharacter::HideOverheadWidget()
{
	if (OverheadWidget)
	{
		OverheadWidget->SetVisibility(false);
	}
}

void ACosmicBlasterCharacter::InteractWithObject()
{
	FHitResult HitResult;

	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = Start + (FollowCamera->GetComponentRotation().Vector() * 450.F);

	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjects;
	TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> FoundCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACosmicBlasterCharacter::StaticClass(), FoundCharacters);

	for (AActor* Actor : FoundCharacters)
	{
		ACosmicBlasterCharacter* BlasterCharacter = Cast<ACosmicBlasterCharacter>(Actor);
		ActorsToIgnore.Add(BlasterCharacter);
	}
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(Combat->EquippedWeapon);
	ActorsToIgnore.Add(Combat->SecondaryWeapon);

	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

	const bool bCanInteract = UKismetSystemLibrary::LineTraceSingleForObjects(
		GetWorld(),
		Start,
		End,
		TraceObjects,
		true,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		HitResult,
		true
	);

	if (bCanInteract)
	{
		if (HitResult.GetActor())
		{
			if (HitResult.GetActor()->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
			{
				IInteractInterface::Execute_InteractWithObject(HitResult.GetActor());
				EquipButtonPressed();
			}
		}
	}
}

ACosmicBlasterCharacter::ACosmicBlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->TargetArmLength = 600.f;
	SpringArm->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("Buff Component"));
	Buff->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("Lag Compensation Component"));

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);

	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/* hit boxes for server side rewind */
	Head = CreateDefaultSubobject<UBoxComponent>(TEXT("Head"));
	Head->SetupAttachment(GetMesh(), FName("Head"));
	HitCollisionBoxes.Add(FName("Head"), Head);

	Pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("Pelvis"));
	Pelvis->SetupAttachment(GetMesh(), FName("Pelvis"));
	HitCollisionBoxes.Add(FName("Pelvis"), Pelvis);

	Spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine_02"));
	Spine_02->SetupAttachment(GetMesh(), FName("Spine_02"));
	HitCollisionBoxes.Add(FName("Spine_02"), Spine_02);

	Spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine_03"));
	Spine_03->SetupAttachment(GetMesh(), FName("Spine_03"));
	HitCollisionBoxes.Add(FName("Spine_03"), Spine_03);

	UpperArm_L = CreateDefaultSubobject<UBoxComponent>(TEXT("UpperArm_L"));
	UpperArm_L->SetupAttachment(GetMesh(), FName("UpperArm_L"));
	HitCollisionBoxes.Add(FName("UpperArm_L"), UpperArm_L);

	UpperArm_R = CreateDefaultSubobject<UBoxComponent>(TEXT("UpperArm_R"));
	UpperArm_R->SetupAttachment(GetMesh(), FName("UpperArm_R"));
	HitCollisionBoxes.Add(FName("UpperArm_R"), UpperArm_R);

	LowerArm_L = CreateDefaultSubobject<UBoxComponent>(TEXT("LowerArm_L"));
	LowerArm_L->SetupAttachment(GetMesh(), FName("LowerArm_L"));
	HitCollisionBoxes.Add(FName("LowerArm_L"), LowerArm_L);

	LowerArm_R = CreateDefaultSubobject<UBoxComponent>(TEXT("LowerArm_R"));
	LowerArm_R->SetupAttachment(GetMesh(), FName("LowerArm_R"));
	HitCollisionBoxes.Add(FName("LowerArm_R"), LowerArm_R);

	Hand_L = CreateDefaultSubobject<UBoxComponent>(TEXT("Hand_L"));
	Hand_L->SetupAttachment(GetMesh(), FName("Hand_L"));
	HitCollisionBoxes.Add(FName("Hand_L"), Hand_L);

	Hand_R = CreateDefaultSubobject<UBoxComponent>(TEXT("Hand_R"));
	Hand_R->SetupAttachment(GetMesh(), FName("Hand_R"));
	HitCollisionBoxes.Add(FName("Hand_R"), Hand_R);

	Backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("Backpack"));
	Backpack->SetupAttachment(GetMesh(), FName("Backpack"));
	HitCollisionBoxes.Add(FName("Backpack"), Backpack);

	Blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("Blanket"));
	Blanket->SetupAttachment(GetMesh(), FName("Backpack"));
	HitCollisionBoxes.Add(FName("Blanket"), Blanket);

	Thigh_L = CreateDefaultSubobject<UBoxComponent>(TEXT("Thigh_L"));
	Thigh_L->SetupAttachment(GetMesh(), FName("Thigh_L"));
	HitCollisionBoxes.Add(FName("Thigh_L"), Thigh_L);

	Thigh_R = CreateDefaultSubobject<UBoxComponent>(TEXT("Thigh_R"));
	Thigh_R->SetupAttachment(GetMesh(), FName("Thigh_R"));
	HitCollisionBoxes.Add(FName("Thigh_R"), Thigh_R);

	Calf_L = CreateDefaultSubobject<UBoxComponent>(TEXT("Calf_L"));
	Calf_L->SetupAttachment(GetMesh(), FName("Calf_L"));
	HitCollisionBoxes.Add(FName("Calf_L"), Calf_L);

	Calf_R = CreateDefaultSubobject<UBoxComponent>(TEXT("Calf_R"));
	Calf_R->SetupAttachment(GetMesh(), FName("Calf_R"));
	HitCollisionBoxes.Add(FName("Calf_R"), Calf_R);

	Foot_L = CreateDefaultSubobject<UBoxComponent>(TEXT("Foot_L"));
	Foot_L->SetupAttachment(GetMesh(), FName("Foot_L"));
	HitCollisionBoxes.Add(FName("Foot_L"), Foot_L);

	Foot_R = CreateDefaultSubobject<UBoxComponent>(TEXT("Foot_R"));
	Foot_R->SetupAttachment(GetMesh(), FName("Foot_R"));
	HitCollisionBoxes.Add(FName("Foot_R"), Foot_R);

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ACosmicBlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnDefaultWeapon();
	UpdateHUDAmmo();
	UpdateHUDHealth();
	UpdateHUDShield();

	GetWorldTimerManager().SetTimer(InteractableTraceTimerHandle, this, &ACosmicBlasterCharacter::ScanForInteractables, 0.25f, true);
	
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ACosmicBlasterCharacter::ReceiveDamage);
	}
	if (BlasterPlayerController)
	{
		BlasterPlayerController->ClearElimText();
	}
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}

}

void ACosmicBlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HideCharacterIfCameraClose();
	PollInit();
	RotateInPlace(DeltaTime);

	if (IsLocallyControlled()) ShowOverheadWidget(this, true);
}

void ACosmicBlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ACosmicBlasterCharacter, OverlappingWeapon, COND_OwnerOnly); //pickup widget only show to the owner of the character (not everyone on the server)
	DOREPLIFETIME(ACosmicBlasterCharacter, Health);
	DOREPLIFETIME(ACosmicBlasterCharacter, bDisableGameplay);
	DOREPLIFETIME(ACosmicBlasterCharacter, Shield);
	DOREPLIFETIME(ACosmicBlasterCharacter, bShouldPlayMacerena);
}

void ACosmicBlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACosmicBlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveForwardController", this, &ACosmicBlasterCharacter::MoveForwardController);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACosmicBlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MoveRightController", this, &ACosmicBlasterCharacter::MoveRightController);
	PlayerInputComponent->BindAxis("Turn", this, &ACosmicBlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("TurnController", this, &ACosmicBlasterCharacter::TurnController);
	PlayerInputComponent->BindAxis("LookUp", this, &ACosmicBlasterCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookUpController", this, &ACosmicBlasterCharacter::LookUpController);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACosmicBlasterCharacter::Jump);
	PlayerInputComponent->BindAction("Swap", IE_Pressed, this, &ACosmicBlasterCharacter::SwapButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ACosmicBlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ACosmicBlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ACosmicBlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACosmicBlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ACosmicBlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ACosmicBlasterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ACosmicBlasterCharacter::GrenadeButtonPressed);
	PlayerInputComponent->BindAction("ToggleLight", IE_Pressed, this, &ACosmicBlasterCharacter::ToggleLightButtonPressed);
}

void ACosmicBlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<ABlasterPlayerController>(Controller);
		}
	}
}

void ACosmicBlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			OnPlayerStateInitialized();

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(BlasterPlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}
	if (BlasterPlayerController == nullptr)
	{
		BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
		if (BlasterPlayerController && Combat && StartingWeapon)
		{
			BlasterPlayerController->SetHUDWeaponType(StartingWeapon->GetWeaponType());
			BlasterPlayerController->SetHUDWeaponAmmo(StartingWeapon->GetAmmo());
			Combat->UpdateCarriedAmmo();
		}
	}
}

void ACosmicBlasterCharacter::OnPlayerStateInitialized()
{
	BlasterPlayerState->AddToScore(0.f); //not setting score/defeats to 0 - adding 0 to the HUD so it is correct on respawn/beginplay
	BlasterPlayerState->AddToDefeats(0);
	SetTeamColour(BlasterPlayerState->GetTeam());
	SetSpawnPoint();
}

void ACosmicBlasterCharacter::HideCharacterIfCameraClose()
{
	if (!IsLocallyControlled()) return;

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
		if (Combat && Combat->EquippedFlag && Combat->EquippedFlag->GetFlagMesh())
		{
			Combat->EquippedFlag->GetFlagMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if (Combat && Combat->EquippedFlag && Combat->EquippedFlag->GetFlagMesh())
		{
			Combat->EquippedFlag->GetFlagMesh()->bOwnerNoSee = false;
		}
	}
}

/*
Teams
*/

void ACosmicBlasterCharacter::SetTeamColour(ETeam Team)
{
	if (GetMesh() == nullptr || OriginalMaterial == nullptr)  return;
	switch (Team)
	{
	case ETeam::ET_NoTeam:
		GetMesh()->SetMaterial(0, OriginalMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;
	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0, BlueMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;
	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0, RedMaterial);
		DissolveMaterialInstance = RedDissolveMatInst;
		break;
	}
}

void ACosmicBlasterCharacter::SetSpawnPoint()
{
	if (HasAuthority() && BlasterPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStarts);
		TArray<ATeamPlayerStart*> TeamPlayerStarts;
		for (auto Start : PlayerStarts)
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			if (TeamStart && TeamStart->Team == BlasterPlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamStart);
			}
		}
		if (TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num() - 1)];
			SetActorLocationAndRotation(ChosenPlayerStart->GetActorLocation(), ChosenPlayerStart->GetActorRotation());
		}
	}
}

/*
Movement functions
*/

void ACosmicBlasterCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if (Controller && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ACosmicBlasterCharacter::MoveForwardController(float Value)
{
	bUsingController = true;
	MoveForward(Value);
}

void ACosmicBlasterCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;
	if (Controller && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ACosmicBlasterCharacter::MoveRightController(float Value)
{
	bUsingController = true;
	MoveRight(Value);
}

void ACosmicBlasterCharacter::Turn(float Value)
{
	if (!Combat->bAiming)
	{
		AddControllerYawInput(Value);
	}
	else
	{
		AddControllerYawInput(Value / 3);
	}
}

void ACosmicBlasterCharacter::LookUp(float Value)
{	
	if (!Combat->bAiming)
	{
		AddControllerPitchInput(Value);
	}
	else
	{
		AddControllerPitchInput(Value / 3);
	}
}

void ACosmicBlasterCharacter::TurnController(float Value)
{
	bUsingController = true;
	if (!Combat->bAiming)
	{
		AddControllerYawInput(Value * 3);
	}
	else
	{
		AddControllerYawInput(Value);
	}
}

void ACosmicBlasterCharacter::LookUpController(float Value)
{
	bUsingController = true;
	if (!Combat->bAiming)
	{
		AddControllerPitchInput(Value * 3);
	}
	else
	{
		AddControllerPitchInput(Value);
	}

}

void ACosmicBlasterCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat && Combat->bHoldingTheFlag) return;
	if (bIsCrouched)
	{
		UnCrouch(); //unreal pre-made function
	}
	else 
	{
		Crouch(); //unreal pre-made function
	}

}

void ACosmicBlasterCharacter::Jump()
{
	if (bDisableGameplay) return;
	if (Combat && Combat->bHoldingTheFlag) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ACosmicBlasterCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ACosmicBlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 5.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ACosmicBlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ACosmicBlasterCharacter::RotateInPlace(float DeltaTime)
{
	if (Combat && Combat->bHoldingTheFlag)
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if (Combat && Combat->EquippedWeapon)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;
	}

	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}

		CalculateAO_Pitch();
	}
}

/*
Aim Offset functions
*/

void ACosmicBlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation); //difference in rotation
		AO_Yaw = DeltaAimRotation.Yaw;

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}

		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ACosmicBlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

float ACosmicBlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

/*
Equip/Weapon functions
*/

void ACosmicBlasterCharacter::SpawnDefaultWeapon()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	UWorld* World = GetWorld();
	if (BlasterGameMode && World && !bElimmed && DefaultWeaponClass)
	{
		StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
			StartingWeapon->SetOwner(this);
		}
	}
}

void ACosmicBlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController && Combat && Combat->EquippedWeapon)
	{
		BlasterPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		BlasterPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void ACosmicBlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (IsLocallyControlled() && OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false, false);
	}

	OverlappingWeapon = Weapon;

	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			if (BlasterPlayerController->bIsInputDeviceGamepad)
			{
				OverlappingWeapon->ShowPickupWidget(true, true);
			}
			else
			{
				OverlappingWeapon->ShowPickupWidget(true, false);
			}

		}
	}
}

void ACosmicBlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (OverlappingWeapon)
	{
		if (BlasterPlayerController->bIsInputDeviceGamepad)
		{
			OverlappingWeapon->ShowPickupWidget(true, true);
		}
		else
		{
			OverlappingWeapon->ShowPickupWidget(true, false);
		}
		
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false, false);
	}
}

void ACosmicBlasterCharacter::AimButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat && Combat->bHoldingTheFlag) return;
	if (Combat && Combat->EquippedWeapon != nullptr)
	{
		Combat->SetAiming(true);
	}
}

void ACosmicBlasterCharacter::AimButtonReleased()
{
	if (bDisableGameplay) return;
	if (Combat && Combat->bHoldingTheFlag) return;
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ACosmicBlasterCharacter::FireButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat && Combat->bHoldingTheFlag) return;
	if (Combat && Combat->EquippedWeapon != nullptr)
	{
		Combat->FireButtonPressed(true);
	}
}

void ACosmicBlasterCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return;
	if (Combat && Combat->bHoldingTheFlag) return;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ACosmicBlasterCharacter::EquipButtonPressed() //for server use
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		if (Combat->bHoldingTheFlag) return;
		if (Combat->CombatState == ECombatState::ECS_Unoccupied)
		{
			ServerEquipButtonPressed();
		}
	}
}

void ACosmicBlasterCharacter::SwapButtonPressed()
{
	if (bDisableGameplay) return;

	bool bSwap = Combat->ShouldSwapWeapons() && Combat->CombatState == ECombatState::ECS_Unoccupied;

	if (bSwap && Combat)
	{
		if (Combat->bHoldingTheFlag) return;

		Combat->SwapWeapons();
	}
}

void ACosmicBlasterCharacter::ServerEquipButtonPressed_Implementation() //for client use
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 8.F, FColor::FromHex("#FFD801"), __FUNCTION__);
			Combat->EquipWeapon(OverlappingWeapon);
		}
	}
} 

bool ACosmicBlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ACosmicBlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

void ACosmicBlasterCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat && Combat->bHoldingTheFlag) return;
	if (Combat)
	{
		Combat->Reload();
	}
}

void ACosmicBlasterCharacter::GrenadeButtonPressed()
{
	if (Combat)
	{
		if (Combat->bHoldingTheFlag) return;
		Combat->ThrowGrenade();
	}
}

void ACosmicBlasterCharacter::ToggleLightButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat && Combat->EquippedWeapon)
	{
		ServerToggleLightButtonPressed();
	}
}

void ACosmicBlasterCharacter::ServerToggleLightButtonPressed_Implementation()
{
	if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->SpotLight)
	{
		if (!Combat->EquippedWeapon->SpotLight->IsVisible())
		{
			Combat->EquippedWeapon->SetSpotLight(true);
		}
		else
		{
			Combat->EquippedWeapon->SetSpotLight(false);
		}
	}
}

/*
Getter functions
*/

AWeapon* ACosmicBlasterCharacter::GetEquippedWeapon()
{
	if(Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector ACosmicBlasterCharacter::GetHitTarget() const
{
	if(Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState ACosmicBlasterCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

bool ACosmicBlasterCharacter::IsLocallyReloading()
{
	if(Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}

bool ACosmicBlasterCharacter::IsHoldingTheFlag() const
{
	if (Combat == nullptr) return false;
	return Combat->bHoldingTheFlag;
}

ETeam ACosmicBlasterCharacter::GetTeam()
{
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if (BlasterPlayerState == nullptr) return ETeam::ET_NoTeam;
	return BlasterPlayerState->GetTeam();;
}

void ACosmicBlasterCharacter::SetHoldingTheFlag(bool bHolding)
{
	if (Combat == nullptr) return;
	Combat->bHoldingTheFlag = bHolding;
}

bool ACosmicBlasterCharacter::ShouldPlayMacerenaMontage()
{
	return bShouldPlayMacerena;
}

ACosmicBlasterCharacter* ACosmicBlasterCharacter::GetCharacter()
{
	return (ACosmicBlasterCharacter*)this;
}

/*
Montage functions
*/

void ACosmicBlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ACosmicBlasterCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage && Combat->CombatState == ECombatState::ECS_Unoccupied)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ACosmicBlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ACosmicBlasterCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Sniper");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Rifle");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ACosmicBlasterCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ACosmicBlasterCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

void ACosmicBlasterCharacter::PlayMacerenaMontage()
{
	PlayMacerena(true);
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && MacerenaMontage)
	{
		AnimInstance->Montage_Play(MacerenaMontage);
	}
}

/*
Damage / Health functions
*/

void ACosmicBlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if (bElimmed || BlasterGameMode == nullptr) return;

	Damage = BlasterGameMode->CalculateDamage(InstigatorController, Controller, Damage);


	float DamageToHealth = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0, MaxShield);
			DamageToHealth = 0.f; //shield took all the damage
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage); //damage to health minus what shield absorped
			Shield = 0.f;
		}
	}

	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);

	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();

	if (Health == 0.f)
	{
		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
}

void ACosmicBlasterCharacter::UpdateHUDShield()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void ACosmicBlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void ACosmicBlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ACosmicBlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}


/*
Elimination / Dissolve effect
*/

void ACosmicBlasterCharacter::Elim(APlayerController* AttackerController, bool bPlayerLeftGame)
{
	bElimmed = true;

	FString AttackerName = FString();
	ABlasterPlayerController* AttackerBlasterController = Cast<ABlasterPlayerController>(AttackerController);
	if (AttackerController)
	{
		ABlasterPlayerState* AttackerBlasterPlayerState = Cast<ABlasterPlayerState>(AttackerBlasterController->PlayerState);
		if (AttackerBlasterPlayerState)
		{
			AttackerName = AttackerBlasterPlayerState->GetPlayerName();
		}
	}

	DropOrDestroyWeapons();
	MulticastElim(AttackerName, bPlayerLeftGame);
}

void ACosmicBlasterCharacter::MulticastElim_Implementation(const FString& AttackerName, bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	bElimmed = true;

	GetCharacterMovement()->DisableMovement();

	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}

	PlayElimMontage();

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
		if (!bPlayerLeftGame)
		{
			BlasterPlayerController->SetElimText(AttackerName);
		}
	}

	//play dissolve material
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), -0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	//disable movement on elimination
	bDisableGameplay = true;

	// disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetVisibility(false);

	//Spawn elim bot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, GetActorLocation());
	}

	bool bHideSniperScope = IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	GetWorldTimerManager().SetTimer(ElimTimer, this, &ACosmicBlasterCharacter::ElimTimerFinished, ElimDelay);
}

void ACosmicBlasterCharacter::ElimTimerFinished()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if (BlasterGameMode && !bLeftGame)
	{
		BlasterGameMode->RequestRespawn(this, Controller);

		if (BlasterPlayerController)
		{
			BlasterPlayerController->ClearElimText();
		}
	}
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void ACosmicBlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void ACosmicBlasterCharacter::DropOrDestroyWeapons()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if (Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
		if (Combat->EquippedFlag)
		{
			Combat->EquippedFlag->DetachfromBackpack();
		}
	}
}

void ACosmicBlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;

	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void ACosmicBlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ACosmicBlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ACosmicBlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

/*
Winning / Crown
*/

void ACosmicBlasterCharacter::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) return;
	if (CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetMesh(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 110.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void ACosmicBlasterCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

/*
Leave game
*/

void ACosmicBlasterCharacter::ServerLeaveGame_Implementation()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if (BlasterGameMode && BlasterPlayerState)
	{
		BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
	}
}

/*
Macerena
*/

void ACosmicBlasterCharacter::OnRep_PlayMacerena()
{
	if(IsLocallyControlled()) bShouldPlayMacerena = bCooldown;
}

void ACosmicBlasterCharacter::ServerPlayMacerena_Implementation(bool bPlayMacerena)
{
	bShouldPlayMacerena = bPlayMacerena;

	//maybe destroy weapon here?
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Destroy();
	}
	/*if (Combat && Combat->EquippedFlag)
	{
		Combat->EquippedFlag->ServerDetachfromBackpack();
	}*/
	if (Combat)
	{
		Combat->ShowAttachedGrenade(false);
	}
}

void ACosmicBlasterCharacter::PlayMacerena(bool bPlayMacerena)
{
	bShouldPlayMacerena = bPlayMacerena;
	ServerPlayMacerena(bPlayMacerena);
	if (IsLocallyControlled())
	{
		bCooldown = bPlayMacerena;
	}
}

void ACosmicBlasterCharacter::PlayFireworks(const TArray<ABlasterPlayerState*>& Players)
{
	if (Players.Num() == 1)
	{
		if (FireworkSystem)
		{
			FireworkComponent = UGameplayStatics::SpawnEmitterAttached(
				FireworkSystem,
				GetRootComponent(),
				FName(),
				Players[0]->GetPawn()->GetActorLocation(),
				GetActorRotation(),
				EAttachLocation::KeepWorldPosition
		);
		}
	}
	else if (Players.Num() > 1)
	{
		for (auto TiedPlayers : Players)
		{
			if (FireworkSystem)
			{
				FireworkComponent = UGameplayStatics::SpawnEmitterAttached(
					FireworkSystem,
					GetRootComponent(),
					FName(),
					TiedPlayers->GetPawn()->GetActorLocation(),
					GetActorRotation(),
					EAttachLocation::KeepWorldPosition
				);
			}
		}

	}
}

void ACosmicBlasterCharacter::PlayFireworkSound(const TArray<ABlasterPlayerState*>& Players)
{
	if (Players.Num() == 1)
	{
		if (FireworkSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireworkSound,
				Players[0]->GetPawn()->GetActorLocation()
		);
		}
	}
	else if (Players.Num() > 1)
	{
		for (auto TiedPlayers : Players)
		{
			if (FireworkSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					FireworkSound,
					TiedPlayers->GetPawn()->GetActorLocation()
				);
			}
		}
	}
}