// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "ShooterGameMode.h"


// Sets default values for this component's properties
UShooterHealthComponent::UShooterHealthComponent()
{
	MaxHealth = 100.0f;

	Health = 100.0f;

	bIsDead = false;

	TeamNum = 255;

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
	if (Damage <= 0 || bIsDead)
	{
		return;
	}

	if (DamageCauser != DamagedActor && IsFriendly(DamageCauser, DamagedActor))
	{
		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

	if (OnHeathChangedEvent.IsBound())
	{
		OnHeathChangedEvent.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
	}
	
	bIsDead = Health <= 0;

	if (bIsDead)
	{
		AShooterGameMode* GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			if (GM->OnActorKilled.IsBound())
			{
				GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Health changed: %s (-%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(Damage));
}

void UShooterHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = Health - OldHealth;

	if (OnHeathChangedEvent.IsBound())
	{
		OnHeathChangedEvent.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
	}
}

void UShooterHealthComponent::Heal(float AmountOfHeal)
{
	if (AmountOfHeal <= 0 || Health <= 0)
	{
		return;
	}

	Health = FMath::Clamp(Health + AmountOfHeal, 0.0f, MaxHealth);

	if (OnHeathChangedEvent.IsBound())
	{
		OnHeathChangedEvent.Broadcast(this, Health, -AmountOfHeal, nullptr, nullptr, nullptr);
	}

	UE_LOG(LogTemp, Log, TEXT("Health changed: %s (+%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(AmountOfHeal));
}

float UShooterHealthComponent::GetHealth() const
{
	return Health;
}

float UShooterHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}

bool UShooterHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		return true;
	}


	UShooterHealthComponent* HealthCompA = Cast<UShooterHealthComponent>(ActorA->GetComponentByClass(UShooterHealthComponent::StaticClass()));
	UShooterHealthComponent* HealthCompB = Cast<UShooterHealthComponent>(ActorB->GetComponentByClass(UShooterHealthComponent::StaticClass()));


	if (HealthCompA == nullptr || HealthCompB == nullptr)
	{
		return true;
	}

	return HealthCompA->TeamNum == HealthCompB->TeamNum;
}

void UShooterHealthComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UShooterHealthComponent, Health);
	DOREPLIFETIME(UShooterHealthComponent, MaxHealth);
}