// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsFlag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "CosmicBlaster/Character/CosmicBlasterCharacter.h"
#include "CosmicBlaster/PlayerState/BlasterPlayerState.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "CosmicBlaster/BlasterTypes/Team.h"
#include "FlagTypes.h"
#include "CosmicBlaster/BlasterComponents/CombatComponent.h"
#include "CosmicBlaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ATeamsFlag::ATeamsFlag()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Flagmesh"));
	SetRootComponent(FlagMesh);

	FlagMesh->SetRelativeScale3D(FVector(.4f, .4f, .4f));

	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetSphereRadius(150.f);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ATeamsFlag::OnSphereOverlap);

}

// Called when the game starts or when spawned
void ATeamsFlag::BeginPlay()
{
	Super::BeginPlay();

	InitialSpawnLocation = GetActorLocation();

	FlagState = EFlagState::EFS_Initial;
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			BindOverlapTimer,
			this,
			&ATeamsFlag::BindOverlapTimerFinished,
			BindOverlapTime
		);
	}
}

void ATeamsFlag::BindOverlapTimerFinished()
{
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ATeamsFlag::OnSphereOverlap);
}

void ATeamsFlag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATeamsFlag, FlagState);
}

void ATeamsFlag::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACosmicBlasterCharacter* BlasterCharacter = Cast<ACosmicBlasterCharacter>(OtherActor);
	if (FlagState == EFlagState::EFS_Equipped) return;
	if (BlasterCharacter)
	{
		if (FlagType == EFlagType::EFT_RedFlag && BlasterCharacter->GetTeam() == ETeam::ET_RedTeam)
		{
			MulticastFlagRespawn();
		}
		else if (FlagType == EFlagType::EFT_BlueFlag && BlasterCharacter->GetTeam() == ETeam::ET_BlueTeam)
		{
			MulticastFlagRespawn();
		}
		else if (FlagType == EFlagType::EFT_RedFlag && BlasterCharacter->GetTeam() == ETeam::ET_BlueTeam)
		{
			BlasterCharacter->GetCombat()->EquipFlag(this);
		}
		else if (FlagType == EFlagType::EFT_BlueFlag && BlasterCharacter->GetTeam() == ETeam::ET_RedTeam)
		{
			BlasterCharacter->GetCombat()->EquipFlag(this);
		}
	}
}

void ATeamsFlag::ServerDetachfromBackpack()
{
	MulticastDetachfromBackpack();

	if (FlagState == EFlagState::EFS_Equipped)
	{
		FlagState = EFlagState::EFS_Dropped;

		FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
		FlagMesh->DetachFromComponent(DetachRules);

		// Set the rotation to 0, 0, 0
		FRotator NewRotation = FRotator::ZeroRotator;
		FlagMesh->SetWorldRotation(NewRotation);

		SetOwner(nullptr);
		OwningCharacter = nullptr;
		OwningController = nullptr;
	}
}

void ATeamsFlag::MulticastDetachfromBackpack_Implementation()
{
	if (FlagState == EFlagState::EFS_Equipped)
	{
		FlagState = EFlagState::EFS_Dropped;

		FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
		FlagMesh->DetachFromComponent(DetachRules);

		// Set the rotation to 0, 0, 0
		FRotator NewRotation = FRotator::ZeroRotator;
		FlagMesh->SetWorldRotation(NewRotation);

		SetOwner(nullptr);
		OwningCharacter = nullptr;
		OwningController = nullptr;
	}
}

void ATeamsFlag::MulticastFlagRespawn_Implementation()
{
	if (GetActorLocation() == InitialSpawnLocation) return;
	SetActorLocation(InitialSpawnLocation);
	FlagState = EFlagState::EFS_Initial;
}

void ATeamsFlag::SetFlagState(EFlagState State)
{
	switch (FlagState)
	{
	case EFlagState::EFS_Initial:
		FlagMesh->SetSimulatePhysics(false);
		FlagMesh->SetEnableGravity(false);
		FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

		OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

		break;
	case EFlagState::EFS_Equipped:
		FlagMesh->SetSimulatePhysics(false);
		FlagMesh->SetEnableGravity(false);

		FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
		OverlapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

		break;
	case EFlagState::EFS_Dropped:
		if (HasAuthority())
		{
			OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		FlagMesh->SetSimulatePhysics(true);
		FlagMesh->SetEnableGravity(true);
		FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

		OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);


		break;
	case EFlagState::EFS_MAX:
		break;
	default:
		break;
	}
}

void ATeamsFlag::OnRep_FlagState()
{
	switch (FlagState)
	{
	case EFlagState::EFS_Initial:
		FlagMesh->SetSimulatePhysics(false);
		FlagMesh->SetEnableGravity(false);
		FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

		OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		break;
	case EFlagState::EFS_Equipped:
		FlagMesh->SetSimulatePhysics(false);
		FlagMesh->SetEnableGravity(false);

		FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
		OverlapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

		break;
	case EFlagState::EFS_Dropped:
		if (HasAuthority())
		{
			OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		FlagMesh->SetSimulatePhysics(true);
		FlagMesh->SetEnableGravity(true);
		FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Overlap);
		FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

		OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		break;
	case EFlagState::EFS_MAX:
		break;
	default:
		break;
	}
}
