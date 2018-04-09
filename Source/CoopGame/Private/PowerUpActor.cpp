// Fill out your copyright notice in the Description page of Project Settings.

#include "PowerUpActor.h"
#include "Net/UnrealNetwork.h"


// Sets default values
APowerUpActor::APowerUpActor()
{
	PowerupInterval = 0.0f;
	MaxNrOfTick = 0;
	TickIterator = 0;

	bIsPowerupActive = false;

	SetReplicates(true);

}


void APowerUpActor::OnRep_PowerupActive()
{
	if (Role < ROLE_Authority)
	{
		if (bIsPowerupActive)
		{
			OnActivated(nullptr);
		}
		else
		{
			OnExpiried();
		}
	}
}


void APowerUpActor::ActivatePowerUp(AActor* ActivateFor)
{
	if (PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerUpTick, this, &APowerUpActor::OnPowerUpTick, PowerupInterval, true);
	}
	else
	{
		OnPowerUpTick();
	}

	OnActivated(ActivateFor);

	bIsPowerupActive = true;
}

void APowerUpActor::OnPowerUpTick()
{
	TickIterator++;

	if (TickIterator >= MaxNrOfTick)
	{
		LastTick();
	}

	OnPowerupTicked();
}


void APowerUpActor::LastTick()
{
	if (bIsPowerupActive)
	{
		OnExpiried();

		bIsPowerupActive = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_PowerUpTick);
	}
}

void APowerUpActor::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APowerUpActor, bIsPowerupActive);
}