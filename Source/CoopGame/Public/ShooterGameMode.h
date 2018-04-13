// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

enum class EWaveState : uint8;


/**
 * 
 */
UCLASS()
class COOPGAME_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	FTimerHandle TimerHandle_SpawnBot;

	FTimerHandle TimerHandle_NextWave;

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

	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetGameState(EWaveState NewState);

public:

	AShooterGameMode();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;

};
