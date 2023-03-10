// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();

	if (World && MuzzleFlashSocket && InstigatorPawn)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();


		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile;
		
		if (bUseServerSideRewind) //bool set on each weapon
		{
			if (InstigatorPawn->HasAuthority()) //server
			{
				if (InstigatorPawn->IsLocallyControlled()) //server, host - use regular replicated projectile
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					if (SpawnedProjectile)
					{
						SpawnedProjectile->bUseServerSideRewind = false;
						SpawnedProjectile->Damage = Damage;
						SpawnedProjectile->HeadShotDamage = HeadShotDamage;
					}
				}
				else //server, not locally controlled - use non-replicated projectile, server side rewind (SSR)
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					if (SpawnedProjectile)
					{
						//UE_LOG(LogTemp, Warning, TEXT("WITH TRACER SSR projectile spawned - server, not locally controlled"));
						SpawnedProjectile->bUseServerSideRewind = true;
						SpawnedProjectile->SetReplicates(true); //replicates projectile down to clients BUT also to the client that is spawning it's own local projectile - sees double
					}

				}
			}
			else //not server (client), using SSR
			{
				if (InstigatorPawn->IsLocallyControlled()) //client, locally controlled - spawn non-replicated projectile, use SSR
				{	//created a duplicate ssr projectile class but removed tracer etc so it's invisible so client only sees one projectile
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindDummyProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					if (SpawnedProjectile)
					{
						//UE_LOG(LogTemp, Warning, TEXT("Dummy no tracer SSR projectile spawned - client, locally controlled"));
						SpawnedProjectile->bUseServerSideRewind = true;
						SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
						SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
					}
				}
				else //client, not locally controlled - use non replicated projectile, no SSR
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					if (SpawnedProjectile)
					{
						SpawnedProjectile->bUseServerSideRewind = false;
					}
				}
			}
		}
		else //weapon not using SSR
		{
			if (InstigatorPawn->HasAuthority()) //server
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				if (SpawnedProjectile)
				{
					SpawnedProjectile->bUseServerSideRewind = false;
					SpawnedProjectile->Damage = Damage;
					SpawnedProjectile->HeadShotDamage = HeadShotDamage;
				}
			}
		}
	}
}
