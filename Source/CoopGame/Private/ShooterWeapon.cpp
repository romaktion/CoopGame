// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame.h"
#include "TimerManager.h"
#include "ShooterCharacter.h"
#include "Net/UnrealNetwork.h"


static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeaponDrawing"),
	DebugWeaponDrawing,
	TEXT("Draw Debug Lines for Weapons"),
	ECVF_Cheat);

// Sets default values
AShooterWeapon::AShooterWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	BaseDamage = 20.0f;
	FireRate = 300.0f;
	RestoreAccuracySpeed = 5.0f;
	SpreadingRate = 1.3f;
	MaxSpreadAngle = 3.5f;
	AccuracyOfGuidance = 0.9f;
	ClipSize = 20;

	SetReplicates(true);
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void AShooterWeapon::BeginPlay()
{
	Super::BeginPlay();

	DelayBetweenShots = 60 / FireRate;

	if (Role == ROLE_Authority)
	{
		CurrentBulletAmount = ClipSize;
	}
}

void AShooterWeapon::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	if (CurrentBulletAmount > 0)
	{
		AActor* MyOwner = GetOwner();

		if (MyOwner)
		{
			AShooterCharacter* MyChar = Cast<AShooterCharacter>(MyOwner);

			if (MyChar && MyChar->bIsReloadingWeapon)
			{
				return;
			}

			FVector EyesLocation;
			FRotator EyesRotation;

			MyOwner->GetActorEyesViewPoint(EyesLocation, EyesRotation);

			FRotator ShootRotation = MeshComp->GetSocketRotation(MuzzleSocketName);
			FVector TraceStart = MeshComp->GetSocketLocation(MuzzleSocketName);

			if (FVector::DotProduct(EyesRotation.Vector(), ShootRotation.Vector()) > AccuracyOfGuidance)
			{
				ShootRotation = EyesRotation;
				TraceStart = EyesLocation;
			}

			ShootRotation.Yaw += FMath::RandRange(-QueueAmount, QueueAmount);
			ShootRotation.Pitch += FMath::RandRange(-QueueAmount, QueueAmount);
			ShootRotation.Roll += FMath::RandRange(-QueueAmount, QueueAmount);

			FVector ShootDirection = ShootRotation.Vector();

			FVector TraceEnd = TraceStart + (ShootDirection * 10000);

			FCollisionQueryParams Queryparams;
			Queryparams.AddIgnoredActor(MyOwner);
			Queryparams.AddIgnoredActor(this);
			Queryparams.bTraceComplex = true;
			Queryparams.bReturnPhysicalMaterial = true;

			FVector TracerTargetLocation = TraceEnd;

			EPhysicalSurface SurfaceType = SurfaceType_Default;

			FHitResult Hit;
			if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, COLLISION_WEAPON, Queryparams))
			{
				AActor* HitActor = Hit.GetActor();
				SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

				float ActualDamage = BaseDamage;
				if (SurfaceType == SURFACE_FLASHVULNARABLE)
				{
					ActualDamage *= 4.0f;
				}

				UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShootDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

				TracerTargetLocation = Hit.ImpactPoint;

				PlayImpactEffect(SurfaceType, TracerTargetLocation);
			}

			if (DebugWeaponDrawing > 0)
			{
				DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
			}

			if (!GetWorldTimerManager().IsTimerActive(TimerHandle_QueueAccum))
			{
				QueueAmount = SpreadingRate;

				GetWorldTimerManager().SetTimer(TimerHandle_QueueAccum, this, &AShooterWeapon::QueueAccum, GetWorld()->DeltaTimeSeconds, true);
			}
			else
			{
				AShooterCharacter* MyOwnerCasted = Cast<AShooterCharacter>(MyOwner);

				float MaxAngle = MaxSpreadAngle;

				if (MyOwnerCasted)
				{
					if (MyOwnerCasted->bWantsToZoom)
					{
						MaxAngle *= 0.5f;
					}

					if (MyOwnerCasted->bIsCrouched)
					{
						MaxAngle *= 0.5f;
					}
				}


				if (QueueAmount < MaxAngle)
				{
					QueueAmount += SpreadingRate;
				}
				else
				{
					QueueAmount -= SpreadingRate;
					
					if (QueueAmount < 0)
					{
						QueueAmount = 0;
					}
				}
			}

			if (WeaponOnFire.IsBound())
			{
				WeaponOnFire.Broadcast();
			}

			PlayShotEffects(TracerTargetLocation);

			if (Role == ROLE_Authority)
			{
				HitScanTrace.TraceTo = TracerTargetLocation;
				HitScanTrace.SurfaceType = SurfaceType;
				CurrentBulletAmount--;
			}

			LastTimeFire = GetWorld()->TimeSeconds;			
		}
	}
}


void AShooterWeapon::ServerFire_Implementation()
{
	Fire();
}

bool AShooterWeapon::ServerFire_Validate()
{
	return true;
}

void AShooterWeapon::ServerReload_Implementation()
{
	Reload();
}

bool AShooterWeapon::ServerReload_Validate()
{
	return true;
}

void AShooterWeapon::QueueAccum()
{
	QueueAmount -= RestoreAccuracySpeed * (GetWorld()->DeltaTimeSeconds);

	if (QueueAmount <= 0)
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_QueueAccum);
	}
}

void AShooterWeapon::StartFire()
{
	if (bAutomaticFire)
	{
		float FirstDelay = FMath::Max(DelayBetweenShots - (GetWorld()->TimeSeconds - LastTimeFire), 0.0f);

	    GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AShooterWeapon::Fire, DelayBetweenShots, true, FirstDelay);
	}
	else
	{
		Fire();
	}
}

void AShooterWeapon::StopFire()
{
	GetWorldTimerManager().PauseTimer(TimerHandle_TimeBetweenShots);
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void AShooterWeapon::Reload()
{
	if (Role == ROLE_Authority)
	{
		CurrentBulletAmount = ClipSize;
	}
	else if (Role < ROLE_Authority)
	{
		ServerReload();
	}
	
}

void AShooterWeapon::PlayShotEffects(FVector TracerTargetLocation)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TracerEffectComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzleLocation);

		if (TracerEffectComp)
		{
			TracerEffectComp->SetVectorParameter(TracerTargetName, TracerTargetLocation);
		}
	}

	if (GetOwner())
	{
		APawn* MyOwner = Cast<APawn>(GetOwner());

		if (MyOwner && MyOwner->IsLocallyControlled())
		{
			APlayerController * PC = Cast<APlayerController>(MyOwner->GetController());

			PC->ClientPlayCameraShake(CameraShakeClass);
		}
	}

	if (CurrentBulletAmount > 0)
	{
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetActorLocation());
		}
	}
	else
	{
		if (BlankClipSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), BlankClipSound, GetActorLocation());
		}
	}
}

void AShooterWeapon::PlayImpactEffect(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem * SelectedImpactEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLASHDEFAULT:
	case SURFACE_FLASHVULNARABLE:
		SelectedImpactEffect = FlashImpacteffect;
		break;
	default:
		SelectedImpactEffect = DefaultImpacteffect;
		break;
	}

	if (SelectedImpactEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FVector ShootDirection = ImpactPoint - MuzzleLocation;
		ShootDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedImpactEffect, ImpactPoint, ShootDirection.Rotation());
	}
}

void AShooterWeapon::OnRep_HitScanTrace()
{
	PlayShotEffects(HitScanTrace.TraceTo);

	PlayImpactEffect(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);

	if (WeaponOnFire.IsBound() && Role < ROLE_Authority)
	{
		WeaponOnFire.Broadcast();
	}
}

void AShooterWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShooterWeapon, HitScanTrace, COND_SkipOwner);
	DOREPLIFETIME(AShooterWeapon, CurrentBulletAmount);
}