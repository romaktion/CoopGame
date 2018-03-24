// Fill out your copyright notice in the Description page of Project Settings.

#include "LauncherWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Projectile.h"



void ALauncherWeapon::Fire()
{
	if (ProjectileClass)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FRotator MuzzleRotation = MeshComp->GetSocketRotation(MuzzleSocketName);
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		APawn* Owner = Cast<APawn>(GetOwner());

		if (Owner)
		{
			SpawnParameters.Instigator = Owner;
		}

		AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParameters);

		Projectile->Controller = SpawnParameters.Instigator->GetController();
	}

	
}
