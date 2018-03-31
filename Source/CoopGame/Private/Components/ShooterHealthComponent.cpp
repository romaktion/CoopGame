// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterHealthComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UShooterHealthComponent::UShooterHealthComponent()
{
	MaxHealth = 100.0f;

	Health = 100.0f;

	SetIsReplicated(true);
}


// Called when the game starts
void UShooterHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();

		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &UShooterHealthComponent::OnHandleTakeAnydamage);
		}
	}	

	Health = FMath::Clamp(Health, 0.0f, MaxHealth);
}

void UShooterHealthComponent::OnHandleTakeAnydamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0)
	{
		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

	if (OnHeathChangedEvent.IsBound())
	{
		OnHeathChangedEvent.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
	}
	
	UE_LOG(LogTemp, Log, TEXT("Health changed: %s"), *FString::SanitizeFloat(Health));
}

void UShooterHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = Health - OldHealth;

	OnHeathChangedEvent.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void UShooterHealthComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UShooterHealthComponent, Health);
	DOREPLIFETIME(UShooterHealthComponent, MaxHealth);
}