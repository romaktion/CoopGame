// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterWeapon.h"
#include "ShooterCharacter.generated.h"

class AShooterWeapon;
class UCameraComponent;
class USpringArmComponent;
class UAnimMontage;
class UShooterHealthComponent;
struct FWeaponData;



UCLASS()
class COOPGAME_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent * CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent * SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UShooterHealthComponent * HealthComp;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void StartCrouch();

	void EndCrouch();

	void StartZoom();

	void StopZoom();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartZoom();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopZoom();

	

	void StartReloadWeapon();

	UFUNCTION(Server, Reliable, WithValidation)
    void ServerReloadWeapon();

	UFUNCTION()
	void OnRep_bIsReloadingWeapon();

	void ReloadWeapon();

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomSpeed;

	float DefaultFOV;

	UPROPERTY(Replicated)
	AShooterWeapon* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AShooterWeapon> DefaultWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	FWeaponData WeaponData;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	UAnimMontage* ReloadAnimMontage;

	FTimerHandle TimerHandle_Reload;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;	

	virtual FVector GetPawnViewLocation() const;

	virtual FRotator GetViewRotation() const;

	UFUNCTION(BlueprintCallable, Category = "Player")
	float GetHealth();

	UFUNCTION(BlueprintCallable, Category = "Player")
	float GetMaxHealth();

	UFUNCTION()
	void OnHealthChanged(UShooterHealthComponent* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StopFire();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bDied;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bWantsToZoom;

	UPROPERTY(ReplicatedUsing = OnRep_bIsReloadingWeapon, BlueprintReadOnly, Category = "Player")
	bool bIsReloadingWeapon;
};
