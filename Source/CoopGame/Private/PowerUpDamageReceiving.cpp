// Fill out your copyright notice in the Description page of Project Settings.

#include "PowerUpDamageReceiving.h"
#include "PowerupSpawnedActor.h"
#include "Net/UnrealNetwork.h"

APowerUpDamageReceiving::APowerUpDamageReceiving()
{
	//APowerUpActor();

	Durability = 100.0f;
	DestroyMultiplier = 1.0f;
	bIsDestroyed = false;
	SpawnOffset = FVector(0, 0, 0);
	SpawnAdditionalRotation = FRotator(0, 0, 0);
	PowerupInterval = 5.0f;
	MaxNrOfTick = 1;

}


void APowerUpDamageReceiving::ActivatePowerUp(AActor* ActivateFor)
{
	FActorSpawnParameters SpawnParam;
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParam.Owner = ActivateFor;

	APawn* CastedPawn = Cast<APawn>(ActivateFor);

	if (CastedPawn)
	{
		SpawnParam.Instigator = CastedPawn;
	}
	

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(ActivateFor->GetActorLocation() + SpawnOffset);
	SpawnTransform.SetRotation(((ActivateFor->GetActorRotation()) + SpawnAdditionalRotation).Quaternion());

	SpawnActorInstance = GetWorld()->SpawnActor<APowerupSpawnedActor>(SpawnActorClass, SpawnTransform, SpawnParam);

	if (SpawnActorInstance)
	{
		SpawnActorInstance->OnTakeAnyDamage.AddDynamic(this, &APowerUpDamageReceiving::HandleTakeDamage);

		Super::ActivatePowerUp(ActivateFor);
	}
}


AActor* APowerUpDamageReceiving::GetSpawnedActor()
{
	return SpawnActorInstance;
}

void APowerUpDamageReceiving::HandleTakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	Durability -= Damage * DestroyMultiplier;

	DamageTaken(DamagedActor, Damage, DamageType, InstigatedBy, DamageCauser);

	if (Durability <= 0)
	{
		LastTick();
	}
}


void APowerUpDamageReceiving::LastTick()
{
	Super::LastTick();

	DestroySpawnerActor();
}


void APowerUpDamageReceiving::OnRep_Durability(float OldDurability)
{
	if (Role < ROLE_Authority)
	{
		float Damage = (Durability - OldDurability) / DestroyMultiplier;

		DamageTaken(SpawnActorInstance, Damage , nullptr, nullptr, nullptr);
	}
}

void APowerUpDamageReceiving::DestroySpawnerActor()
{
	if (!bIsDestroyed)
	{
		SpawnActorInstance->Destroy(true);

		bIsDestroyed = true;

		this->Destroy(true);
	}
}

void APowerUpDamageReceiving::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APowerUpDamageReceiving, Durability);
}