// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	FTimerHandle TimerHandle_SpawnBot;

	int32 NbOfBotsToSpawn;

	int32 WaveCounter;

	UPROPERTY(EditDefaultsOnly, Category = "GameMod")
	float TimeBetweenWaves;


protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	void SpawnBottimerElapsed();
	
	void StartWave();

	void EndWave();

	void PrepareForNextWave();

public:

	AShooterGameMode();

	virtual void StartPlay() override;


};
