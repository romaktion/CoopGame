// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGameState.h"
#include "Net/UnrealNetwork.h"




void AShooterGameState::OnRep_WaveState(EWaveState OldState)
{
	WaveStateChanged(OldState, WaveState);
}


void AShooterGameState::SetWaveState(EWaveState NewState)
{
	if (Role == ROLE_Authority)
	{
		EWaveState OldState = WaveState;

		WaveState = NewState;

		OnRep_WaveState(OldState);
	}
}

void AShooterGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterGameState, WaveState);
}