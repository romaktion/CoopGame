// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, UShooterHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API UShooterHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UShooterHealthComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HealthComponent")
	uint8 TeamNum;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	bool bIsDead;

	UPROPERTY(ReplicatedUsing = OnRep_Health, EditDefaultsOnly, Category = "HealthComponent")
	float Health;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "HealthComponent")
	float MaxHealth;

	UFUNCTION()
	void OnHandleTakeAnydamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHeathChangedEvent;

	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
	void Heal(float AmountOfHeal);

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HealthComponent")
	static bool IsFriendly(AActor* ActorA, AActor* ActorB);
};
