// Fill out your copyright notice in the Description page of Project Settings.

#include "TrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "ShooterHealthComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SphereComponent.h"
#include "ShooterCharacter.h"
#include "Sound/SoundCue.h"


// Sets default values
ATrackerBot::ATrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	RootComponent = MeshComp;
	MeshComp->SetSimulatePhysics(true);

	HealthComp = CreateDefaultSubobject<UShooterHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHeathChangedEvent.AddDynamic(this, &ATrackerBot::HandleTakeDamage);

	DetectionRadius = 200;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetSphereRadius(DetectionRadius);
	SphereComp->SetupAttachment(RootComponent);

	AcceptanceRadius = 100;
	ForceValue = 1000;
	bUseVelocityChange = true;

	ExplosionDamage = 40;
	ExplosionRadius = 200;

}

// Called when the game starts or when spawned
void ATrackerBot::BeginPlay()
{
	Super::BeginPlay();
	
	NextPathPoint = GetNextPathPoint();
}

FVector ATrackerBot::GetNextPathPoint()
{
	//Hack
	ACharacter* GoalPawn = UGameplayStatics::GetPlayerCharacter(this, 0);

	UNavigationPath* Path = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), GoalPawn);

	if (Path->PathPoints.Num() > 1)
	{
		return Path->PathPoints[1];
	}

	//Failed
	return GetActorLocation();

}

void ATrackerBot::HandleTakeDamage(UShooterHealthComponent* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    //Explode on health == 0

	//Pulse material on take damage
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}


	if (Health <= 0)
	{
		SelfDestruct();
	}
}

void ATrackerBot::SelfDestruct()
{
	if (bExploded)
	{
		return;
	}

	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);

	UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoreActors, this, GetInstigatorController(), true);

	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	Destroy();
}

void ATrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (!bStartedSelfDestruction)
	{
		if (OtherActor)
		{
			AShooterCharacter* Char = Cast<AShooterCharacter>(OtherActor);

			if (Char)
			{
				GetWorldTimerManager().SetTimer(TimerHandle_DamageSelf, this, &ATrackerBot::DamageSelf, 0.5f, true, 0.0f);

				bStartedSelfDestruction = true;

				UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
			}
		}
	}
}

void ATrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

// Called every frame
void ATrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Distance = (NextPathPoint - GetActorLocation()).Size();

	if (Distance > AcceptanceRadius)
	{
		FVector Direction = NextPathPoint - GetActorLocation();
		Direction.Normalize();

		Direction *= ForceValue;

		MeshComp->AddForce(Direction, NAME_None, bUseVelocityChange);

		DrawDebugSphere(GetWorld(), NextPathPoint, AcceptanceRadius / 4, 12, FColor::Yellow, false, 0);
	}
	else
	{
		NextPathPoint = GetNextPathPoint();
	}

}

