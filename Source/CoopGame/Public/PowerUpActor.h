// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerUpActor.generated.h"

UCLASS()
class COOPGAME_API APowerUpActor : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	APowerUpActor();

	UPROPERTY(EditDefaultsOnly, Category = "PowerUps")
	float PowerupInterval;

	UPROPERTY(EditDefaultsOnly, Category = "PowerUps")
	int32 MaxNrOfTick;

	FTimerHandle TimerHandle_PowerUpTick;

	int32 TickIterator;

	UFUNCTION()
	void OnPowerUpTick();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	void ActivatePowerUp();

	UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
	void OnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
	void OnExpiried();

	UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
	void OnPowerupTicked();
	
};
