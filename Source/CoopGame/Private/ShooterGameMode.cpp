// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGameMode.h"
#include "ShooterGameState.h"
#include "ShooterPlayerState.h"
#include "ShooterHealthComponent.h"


AShooterGameMode::AShooterGameMode()
{
	GameStateClass = AShooterGameState::StaticClass();
	PlayerStateClass = AShooterPlayerState::StaticClass();

	TimeBetweenWaves = 2.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
}




void AShooterGameMode::StartWave()
{
	WaveCounter++;

	NbOfBotsToSpawn = 2 * WaveCounter;


	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBot, this, &AShooterGameMode::SpawnBottimerElapsed, 1.0f, true, 0.0f);


	SetWaveState(EWaveState::WaveInProgress);
}

void AShooterGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_SpawnBot);

	SetWaveState(EWaveState::WaitingToComplete);
}

void AShooterGameMode::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWave, this, &AShooterGameMode::StartWave, TimeBetweenWaves, false);

	SetWaveState(EWaveState::WaitingToStart);
}

void AShooterGameMode::CheckWaveState()
{
	bool bIsPreparingForNextWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWave);

	if (NbOfBotsToSpawn > 0 || bIsPreparingForNextWave)
	{
		return;
	}


	bool bIsAnyAliveBot = false;


	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();

		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}

		UShooterHealthComponent* HealthComp = Cast<UShooterHealthComponent>(TestPawn->GetComponentByClass(UShooterHealthComponent::StaticClass()));

		if (HealthComp)
		{
			if (HealthComp->GetHealth() > 0)
			{
				bIsAnyAliveBot = true;
				break;
			}
		}
	}

	if (!bIsAnyAliveBot)
	{
		SetWaveState(EWaveState::WaveComplete);

		PrepareForNextWave();
	}

}

void AShooterGameMode::CheckAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();


		if (PC && PC->GetPawn())
		{
			APawn* TestPawn = PC->GetPawn();

			UShooterHealthComponent* HealthComp = Cast<UShooterHealthComponent>(TestPawn->GetComponentByClass(UShooterHealthComponent::StaticClass()));

			if (ensure(HealthComp))
			{
				if (HealthComp->GetHealth() > 0.0f)
				{
					return;
				}
			}
		}
	}

	GameOver();
}

void AShooterGameMode::GameOver()
{
	EndWave();

	SetWaveState(EWaveState::GameOver);

	UE_LOG(LogTemp, Log, TEXT("GAME OVER! All Player Died"));
}

void AShooterGameMode::SetWaveState(EWaveState NewState)
{
	AShooterGameState* GS = GetGameState<AShooterGameState>();

	if (ensureAlways(GS))
	{
		GS->SetWaveState(NewState);
	}
}

void AShooterGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();


}

void AShooterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();

	CheckAnyPlayerAlive();
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
