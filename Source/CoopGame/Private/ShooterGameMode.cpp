// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGameMode.h"


AShooterGameMode::AShooterGameMode()
{
	TimeBetweenWaves = 2.0f;
}




void AShooterGameMode::StartWave()
{
	WaveCounter++;

	NbOfBotsToSpawn = 2 * WaveCounter;


	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBot, this, &AShooterGameMode::SpawnBottimerElapsed, 1.0f, true, 0.0f);

}

void AShooterGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_SpawnBot);

	PrepareForNextWave();
}

void AShooterGameMode::PrepareForNextWave()
{
	FTimerHandle TimerHandle_NextWave;

	GetWorldTimerManager().SetTimer(TimerHandle_NextWave, this, &AShooterGameMode::StartWave, TimeBetweenWaves, false);
}

void AShooterGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();


}

void AShooterGameMode::SpawnBottimerElapsed()
{
	SpawnNewBot();

	NbOfBotsToSpawn--;

	if (NbOfBotsToSpawn <= 0)
	{
		EndWave();
	}
}
