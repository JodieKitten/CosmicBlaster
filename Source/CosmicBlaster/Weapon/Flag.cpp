// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "CosmicBlaster/Character/CosmicBlasterCharacter.h"
#include "CosmicBlaster/BlasterComponents/CombatComponent.h"

AFlag::AFlag()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Flag Mesh"));
	SetRootComponent(FlagMesh);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Overlap Sphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	GetBoxCollision()->SetupAttachment(FlagMesh);
	GetPickupWidget()->SetupAttachment(FlagMesh);

	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFlag::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

void AFlag::ResetFlag()
{
	ACosmicBlasterCharacter* FlagBearer = Cast<ACosmicBlasterCharacter>(GetOwner());
	if (FlagBearer)
	{
		FlagBearer->SetHoldingTheFlag(false);
		FlagBearer->SetOverlappingWeapon(nullptr);
		FlagBearer->UnCrouch();
	}

	/*FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);*/

	if (!HasAuthority()) return;
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);

	SetWeaponState(EWeaponState::EWS_Initial);

	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	GetBoxCollision()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetBoxCollision()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetBoxCollision()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;

	SetActorTransform(InitialTransform);
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();

	InitialTransform = GetActorTransform();
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlag::OnSphereOverlap);
}

void AFlag::OnEquipped()
{
	ShowPickupWidget(false);

	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	GetBoxCollision()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetBoxCollision()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

	EnableCustomDepth(false);
}

void AFlag::OnDropped()
{
	if (HasAuthority())
	{
		GetBoxCollision()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	}
	FlagMesh->SetSimulatePhysics(true);
	FlagMesh->SetEnableGravity(true);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	FlagMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
}

void AFlag::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACosmicBlasterCharacter* BlasterCharacter = Cast<ACosmicBlasterCharacter>(OtherActor);
	if (BlasterCharacter && BlasterCharacter->GetCombat())
	{
		if (this->ActorHasTag("Blue") && BlasterCharacter->GetTeam() == ETeam::ET_BlueTeam) return;
		if (this->ActorHasTag("Red") && BlasterCharacter->GetTeam() == ETeam::ET_RedTeam) return;
		if (BlasterCharacter->IsHoldingTheFlag()) return;
		//BlasterCharacter->GetCombat()->EquipFlag(this);
	}
}

void AFlag::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}