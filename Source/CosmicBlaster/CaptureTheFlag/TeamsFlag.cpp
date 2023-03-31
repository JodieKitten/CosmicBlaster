// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsFlag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "CosmicBlaster/Character/CosmicBlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CosmicBlaster/PlayerController/BlasterPlayerController.h"
#include "CosmicBlaster/BlasterTypes/Team.h"
#include "FlagTypes.h"
#include "CosmicBlaster/BlasterComponents/CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "CosmicBlaster/BlasterComponents/BuffComponent.h"

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
	FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	FlagMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetSphereRadius(150.f);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

}

void ATeamsFlag::BeginPlay()
{
	Super::BeginPlay();

	InitialSpawnLocation = GetActorTransform();

	FlagState = EFlagState::EFS_Initial;
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
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
			if (BlasterCharacter->bElimmed) return;

			BlasterCharacter->GetCombat()->EquipFlag(this);

		}
		else if (FlagType == EFlagType::EFT_BlueFlag && BlasterCharacter->GetTeam() == ETeam::ET_RedTeam)
		{
			if (BlasterCharacter->bElimmed) return;

			BlasterCharacter->GetCombat()->EquipFlag(this);
		}
	}
}

void ATeamsFlag::DetachfromBackpack()
{
	if (FlagState == EFlagState::EFS_Equipped)
	{
		SetFlagState(EFlagState::EFS_Dropped);
		//FlagState = EFlagState::EFS_Dropped;

		FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
		FlagMesh->DetachFromComponent(DetachRules);

		// Set the rotation to 0, 0, 0
		FRotator NewRotation = FRotator::ZeroRotator;
		FlagMesh->SetWorldRotation(NewRotation);

		SetOwner(nullptr);
		OwningCharacter = nullptr;
		OwningController = nullptr;
	}

	MulticastDetachfromBackpack();
}

void ATeamsFlag::MulticastDetachfromBackpack_Implementation()
{
	if (FlagState == EFlagState::EFS_Equipped)
	{
		SetFlagState(EFlagState::EFS_Dropped);

		FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
		FlagMesh->DetachFromComponent(DetachRules);

		SetOwner(nullptr);
		OwningCharacter = nullptr;
		OwningController = nullptr;
	}
}

void ATeamsFlag::MulticastFlagRespawn_Implementation()
{
	if (GetActorLocation() == InitialSpawnLocation.GetLocation()) return;

	SetActorTransform(InitialSpawnLocation);
	SetFlagState(EFlagState::EFS_Initial);
}

void ATeamsFlag::SetFlagState(EFlagState State)
{
	FlagState = State;

	switch (FlagState)
	{
	case EFlagState::EFS_Initial:
		OnInitial();
		break;
	case EFlagState::EFS_Equipped:
		OnEquipped();
		break;
	case EFlagState::EFS_Dropped:
		OnDropped();
		break;
	case EFlagState::EFS_MAX:
		break;
	}
}

void ATeamsFlag::OnRep_FlagState()
{
	SetFlagState(FlagState);
}

void ATeamsFlag::OnInitial()
{
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	EnableCustomDepth(true);

	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

void ATeamsFlag::OnEquipped()
{
	SetOwner(OwningCharacter);
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	EnableCustomDepth(false);

	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	OwningCharacter = Cast<ACosmicBlasterCharacter>(GetOwner());
	if (OwningCharacter)
	{
		OwningCharacter->GetCharacterMovement()->MaxWalkSpeed = EquippedFlagSpeed;
	}
}

void ATeamsFlag::OnDropped()
{
	OwningCharacter = Cast<ACosmicBlasterCharacter>(GetOwner());
	if (OwningCharacter)
	{
		OwningCharacter->GetBuff()->ResetSpeeds();
	}
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
	EnableCustomDepth(true);

	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	SetOwner(nullptr);
}

void ATeamsFlag::EnableCustomDepth(bool bEnable)
{
	if (FlagMesh)
	{
		FlagMesh->SetRenderCustomDepth(bEnable);
	}
}
