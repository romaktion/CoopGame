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
#include "Net/UnrealNetwork.h"



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

	DetectionSelfDestroyRadius = 200;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetSphereRadius(DetectionSelfDestroyRadius);
	SphereComp->SetupAttachment(RootComponent);

	AcceptanceRadius = 300;
	ForceValue = 1500;
	bUseVelocityChange = true;

	ExplosionDamage = 40;
	ExplosionRadius = 300;
	SightRadius = 2500;
	DetectAIRadius = 500;
	PowerLevel = 0;
	MaxPowerLevel = 5;

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ATrackerBot::BeginPlay()
{
	Super::BeginPlay();
	if (Role == ROLE_Authority)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_SearchEnemy, this, &ATrackerBot::SearchEnemy, 1.0f, true);

		NextPathPoint = GetNextPathPoint(CurrentEnemy);
	}
}

FVector ATrackerBot::GetNextPathPoint(AActor* GoalActor)
{
	if (GoalActor)
	{
		UNavigationPath* Path = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), GoalActor);

		if (Path->PathPoints.Num() > 1)
		{
			return Path->PathPoints[1];
		}
	}


	//Failed
	return GetActorLocation();
}

bool ATrackerBot::GetEnemy(AActor* &Enemy)
{
	TArray<AActor*> OverlapedActors;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjctTypes;

	TEnumAsByte<EObjectTypeQuery> ObjctType = EObjectTypeQuery::ObjectTypeQuery3;

	ObjctTypes.Add(ObjctType);

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);

	ACharacter* Char;

	UClass* FilterClass = Char->StaticClass();

	OverlapedActors.Empty();

	UKismetSystemLibrary::SphereOverlapActors(this, GetActorLocation(), SightRadius, ObjctTypes, FilterClass, IgnoreActors, OverlapedActors);

	if (OverlapedActors.Num() > 0)
	{
		Enemy = OverlapedActors[0];

		if (OverlapedActors.Num() > 1)
		{
			float minDist = (OverlapedActors[0]->GetActorLocation() - GetActorLocation()).Size();

			for (size_t i = 1; i < OverlapedActors.Num(); i++)
			{
				if ((OverlapedActors[i]->GetActorLocation() - GetActorLocation()).Size() < minDist)
				{
					ACharacter* MyChar = Cast<ACharacter>(OverlapedActors[i]);

					if (MyChar)
					{
						minDist = (OverlapedActors[i]->GetActorLocation() - GetActorLocation()).Size();
						Enemy = OverlapedActors[i];
					}
				}
			}
		}

		return true;
	}

	return false;
}

void ATrackerBot::HandleTakeDamage(UShooterHealthComponent* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    //Explode on health == 0
	if (!bExploded)
	{
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
}

void ATrackerBot::SelfDestruct()
{
	if (bExploded)
	{
		return;
	}

	bExploded = true;

	MeshComp->SetVisibility(false);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	if (Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(this);

		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage * FMath::Clamp(PowerLevel, 1, PowerLevel), GetActorLocation(), ExplosionRadius, nullptr, IgnoreActors, this, GetInstigatorController(), true);

		SetLifeSpan(2.0f);
	}
}

void ATrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (!bStartedSelfDestruction)
	{
		if (OtherActor)
		{
			AShooterCharacter* Char = Cast<AShooterCharacter>(OtherActor);

			if (Char)
			{
				if (Role == ROLE_Authority)
				{
					GetWorldTimerManager().SetTimer(TimerHandle_DamageSelf, this, &ATrackerBot::DamageSelf, 0.5f, true, 0.0f);
				}

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

void ATrackerBot::SearchEnemy()
{
	if (!bExploded)
	{
		AActor* Enemy = nullptr;

		if (GetEnemy(Enemy))
		{
			if (!CurrentEnemy)
			{
				CurrentEnemy = Enemy;

				NextPathPoint = GetNextPathPoint(CurrentEnemy);
			}
			else if (CurrentEnemy != Enemy)
			{
				CurrentEnemy = Enemy;

				NextPathPoint = GetNextPathPoint(CurrentEnemy);
			}
		}
		else
		{
			NextPathPoint = GetActorLocation();
		}

		TArray<FOverlapResult> OverlapResults;
		FCollisionObjectQueryParams QueryParams;
		QueryParams.AddObjectTypesToQuery(ECC_Pawn);
		QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
		FCollisionShape CollShape;
		CollShape.SetSphere(DetectAIRadius);

		GetWorld()->OverlapMultiByObjectType(OverlapResults, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);

		PowerLevel = 0;

		for (FOverlapResult Result : OverlapResults)
		{
			ATrackerBot* Bot = Cast<ATrackerBot>(Result.GetActor());

			if (Bot && Bot != this)
			{
				PowerLevel++;
			}
		}

		PowerLevel = FMath::Clamp(PowerLevel, 0, MaxPowerLevel);

		if (PowerLevel > 0)
		{
			if (MatInst == nullptr)
			{
				MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
			}

			if (MatInst)
			{
				float Alpha = PowerLevel / (float)MaxPowerLevel;


				MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
			}
		}

	}
}

void ATrackerBot::OnRep_PowerLevel()
{
	if (Role < ROLE_Authority)
	{
		if (PowerLevel > 0)
		{
			if (MatInst == nullptr)
			{
				MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
			}

			if (MatInst)
			{
				float Alpha = PowerLevel / (float)MaxPowerLevel;


				MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
			}
		}
	}
}

// Called every frame
void ATrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !bExploded)
	{
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
			NextPathPoint = GetNextPathPoint(CurrentEnemy);
		}
	}
}


void ATrackerBot::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATrackerBot, PowerLevel);
}