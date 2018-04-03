// Fill out your copyright notice in the Description page of Project Settings.

#include "PowerUpActor.h"


// Sets default values
APowerUpActor::APowerUpActor()
{
	PowerupInterval = 0.0f;
	MaxNrOfTick = 0;
	TickIterator = 0;
}


// Called when the game starts or when spawned
void APowerUpActor::BeginPlay()
{
	Super::BeginPlay();
}


void APowerUpActor::ActivatePowerUp()
{
	if (PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerUpTick, this, &APowerUpActor::OnPowerUpTick, PowerupInterval, true);
	}
	else
	{
		OnPowerUpTick();
	}

	OnActivated();
}

void APowerUpActor::OnPowerUpTick()
{
	TickIterator++;

	if (TickIterator >= MaxNrOfTick)
	{
		OnExpiried();

		GetWorldTimerManager().ClearTimer(TimerHandle_PowerUpTick);
	}

	OnPowerupTicked();
}
