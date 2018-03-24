// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWeaponFireSignature);

USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;
};


UCLASS()
class COOPGAME_API AShooterWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShooterWeapon();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* DefaultImpacteffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* FlashImpacteffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> CameraShakeClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool bAutomaticFire;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (EditCondition="bAutomaticFire"))
	float FireRate;

	float DelayBetweenShots;

	FTimerHandle TimerHandle_TimeBetweenShots;

	FTimerHandle TimerHandle_QueueAccum;

	float LastTimeFire;

	float QueueAmount;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RestoreAccuracySpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float MaxSpreadAngle;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float SpreadingRate;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float AccuracyOfGuidance;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	USoundBase* FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	USoundBase* BlankClipSound;

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

	void PlayShotEffects(FVector TracerTargetLocation);

	void PlayImpactEffect(EPhysicalSurface SurfaceType, FVector ImpactPoint);

    virtual void Fire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReload();

	void QueueAccum();

	virtual void BeginPlay() override;

public:

	void StartFire();

	void StopFire();

	void Reload();

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FWeaponFireSignature WeaponOnFire;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	int CurrentBulletAmount;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int ClipSize;

};
