// Fill out your copyright notice in the Description page of Project Settings.

#include "TrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "ShooterHealthComponent.h"


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

	AcceptanceRadius = 100;
	ForceValue = 1000;
	bUseVelocityChange = true;

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

