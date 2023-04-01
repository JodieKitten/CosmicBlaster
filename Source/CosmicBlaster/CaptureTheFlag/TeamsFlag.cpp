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

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Flag Mesh"));
	SetRootComponent(Mesh);

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Overlap Sphere"));
	Sphere->SetupAttachment(RootComponent);

	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);


	Mesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	Mesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
}

void ATeamsFlag::BeginPlay()
{
	Super::BeginPlay();

	InitialSpawnTransform = GetActorTransform();
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ATeamsFlag::OnSphereOverlap);
}

void ATeamsFlag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ATeamsFlag, FlagState);
}

void ATeamsFlag::EnableCustomDepth(bool bEnable)
{
	if (Mesh)
	{
		Mesh->SetRenderCustomDepth(bEnable);
	}
}

void ATeamsFlag::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACosmicBlasterCharacter* Character = Cast<ACosmicBlasterCharacter>(OtherActor);
	if (Character)
	{
		if (FlagType == EFlagType::EFT_RedFlag && Character->GetTeam() == ETeam::ET_RedTeam)
		{
			RespawnFlag();
			//hide red point
		}
		else if (FlagType == EFlagType::EFT_BlueFlag && Character->GetTeam() == ETeam::ET_BlueTeam)
		{
			RespawnFlag();
			//hide blue point
		}
		if (FlagType == EFlagType::EFT_RedFlag && Character->GetTeam() != ETeam::ET_RedTeam)
		{
			Character->GetCombat()->EquipFlag(this);
			//show red point
		}
		else if (FlagType == EFlagType::EFT_BlueFlag && Character->GetTeam() != ETeam::ET_BlueTeam)
		{
			Character->GetCombat()->EquipFlag(this);
			//show blue point
		}
	}
}

void ATeamsFlag::SetFlagState(EFlagState State)
{
	FlagState = State;
	OnFlagStateSet();
}

void ATeamsFlag::OnFlagStateSet()
{
	switch (FlagState)
	{
	case EFlagState::EFS_Equipped:
		Mesh->SetSimulatePhysics(false);
		Mesh->SetEnableGravity(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
		Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		EnableCustomDepth(false);
		break;
	case EFlagState::EFS_Dropped:
		Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Sphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		EnableCustomDepth(true);
		break;
	}
}

void ATeamsFlag::OnRep_FlagState()
{
	OnFlagStateSet();
}

void ATeamsFlag::MulticastDropped_Implementation()
{
	SetFlagState(EFlagState::EFS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	Mesh->DetachFromComponent(DetachRules);

	FRotator NewRotation = FRotator::ZeroRotator;
	Mesh->SetWorldRotation(NewRotation);

	SetOwner(nullptr);
	OwningCharacter = nullptr;
	OwningController = nullptr;
}

void ATeamsFlag::RespawnFlag()
{
	SetFlagState(EFlagState::EFS_Initial);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	Mesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	OwningCharacter = nullptr;
	OwningController = nullptr;
	SetActorTransform(InitialSpawnTransform);
	EnableCustomDepth(true);
	Sphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}
