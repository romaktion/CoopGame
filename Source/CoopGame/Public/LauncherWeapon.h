// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterWeapon.h"
#include "LauncherWeapon.generated.h"

class AProjectile;


/**
 * 
 */
UCLASS()
class COOPGAME_API ALauncherWeapon : public AShooterWeapon
{
	GENERATED_BODY()
	
public:

	virtual void Fire() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AProjectile> ProjectileClass;
};
