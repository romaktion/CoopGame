// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PowerUpActor.h"
#include "PowerUpDamageReceiving.generated.h"


class APowerupSpawnedActor;

/**
 * 
 */
UCLASS()
class COOPGAME_API APowerUpDamageReceiving : public APowerUpActor
{
	GENERATED_BODY()
	
public:
	APowerUpDamageReceiving();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "PowerUps")
	TSubclassOf<APowerupSpawnedActor> SpawnActorClass;

	UFUNCTION()
	void HandleTakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	void DestroySpawnerActor();

	virtual void LastTick() override;

	UPROPERTY(ReplicatedUsing=OnRep_Durability, EditDefaultsOnly, Category = "PowerUps")
	float Durability;

	UFUNCTION()
	void OnRep_Durability(float OldDurability);

	bool bIsDestroyed;

	UPROPERTY(EditDefaultsOnly, Category = "PowerUps")
	FVector SpawnOffset;

	UPROPERTY(EditDefaultsOnly, Category = "PowerUps")
	FRotator SpawnAdditionalRotation;

	UPROPERTY(EditDefaultsOnly, Category = "PowerUps")
	float DestroyMultiplier;

	AActor* SpawnActorInstance;

public:

	virtual void ActivatePowerUp(AActor* ActivateFor) override;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
	void DamageTaken(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "PowerUps")
	AActor* GetSpawnedActor();
};
