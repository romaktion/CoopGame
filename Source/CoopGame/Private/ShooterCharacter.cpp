// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ShooterWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "ShooterHealthComponent.h"
#include "CoopGame.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmcomp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	GetCapsuleComponent()->SetCollisionProfileName("CharacterCapsule");

	HealthComp = CreateDefaultSubobject<UShooterHealthComponent>(TEXT("HealthComp"));

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	ZoomFOV = 65.0f;
	ZoomSpeed = 20.0f;

	WeaponSocketName = "WeaponSocket";
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();


	DefaultFOV = CameraComp->FieldOfView;
	HealthComp->OnHeathChangedEvent.AddDynamic(this, &AShooterCharacter::OnHealthChanged);

	if (Role == ROLE_Authority)
	{
		FActorSpawnParameters SpawnParametrs;
		SpawnParametrs.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParametrs.Owner = this;

		CurrentWeapon = GetWorld()->SpawnActor<AShooterWeapon>(DefaultWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParametrs);

		if (CurrentWeapon)
		{
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
		}
	}
}

void AShooterCharacter::StartCrouch()
{
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		Crouch();
	}
}

void AShooterCharacter::EndCrouch()
{
	UnCrouch();
}

void AShooterCharacter::StartZoom()
{
	if (Role == ROLE_Authority)
	{
		bWantsToZoom = true;
	}
	else if (Role < ROLE_Authority)
	{
		ServerStartZoom();
	}
}

void AShooterCharacter::StopZoom()
{
	if (Role == ROLE_Authority)
	{
		bWantsToZoom = false;
	}
	else if (Role < ROLE_Authority)
	{
		ServerStopZoom();
	}
}

void AShooterCharacter::ServerStartZoom_Implementation()
{
	StartZoom();
}

bool AShooterCharacter::ServerStartZoom_Validate()
{
	return true;
}

void AShooterCharacter::ServerStopZoom_Implementation()
{
	StopZoom();
}

bool AShooterCharacter::ServerStopZoom_Validate()
{
	return true;
}

void AShooterCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		if (CurrentWeapon->CurrentBulletAmount > 0 &&
			!GetWorldTimerManager().IsTimerActive(TimerHandle_Reload))
		{
			CurrentWeapon->StartFire();
		}
		else
		{
			StartReloadWeapon();
		}
	}
}

void AShooterCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();

		if (CurrentWeapon->CurrentBulletAmount <= 0)
		{
			StartReloadWeapon();
		}
	}
}

void AShooterCharacter::StartReloadWeapon()
{
	if (Role < ROLE_Authority)
	{
		ServerReloadWeapon();
	}

	if (CurrentWeapon)
	{
		if (CurrentWeapon->CurrentBulletAmount < CurrentWeapon->ClipSize &&
			!GetWorldTimerManager().IsTimerActive(TimerHandle_Reload))
		{
			if (Role == ROLE_Authority)
			{
				bIsReloadingWeapon = true;
			}

			FName SectionName = "Default";

			float delay = 0;

			if (ReloadAnimMontage)
			{
				delay = PlayAnimMontage(ReloadAnimMontage, 1.0f, SectionName);
			}

			GetWorldTimerManager().SetTimer(TimerHandle_Reload, this, &AShooterCharacter::ReloadWeapon, delay, false);
		}
	}
}

void AShooterCharacter::ServerReloadWeapon_Implementation()
{
	StartReloadWeapon();
}

bool AShooterCharacter::ServerReloadWeapon_Validate()
{
	return true;
}

void AShooterCharacter::OnRep_bIsReloadingWeapon()
{
	if (bIsReloadingWeapon && ReloadAnimMontage && !IsLocallyControlled() && Role < ROLE_Authority)
	{
		FName SectionName = "Default";

		PlayAnimMontage(ReloadAnimMontage, 1.0f, SectionName);
	}
}

void AShooterCharacter::ReloadWeapon()
{
	if (Role == ROLE_Authority)
	{
		bIsReloadingWeapon = false;
	}

	GetWorldTimerManager().ClearTimer(TimerHandle_Reload);

	if (CurrentWeapon)
	{
		CurrentWeapon->Reload();
	}
}

void AShooterCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}

void AShooterCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Zoom
	float TargetFOV = bWantsToZoom ? ZoomFOV : DefaultFOV;
	float NewFov = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomSpeed);

	CameraComp->SetFieldOfView(NewFov);

	//Replicate pitch

	if (!IsLocallyControlled())
	{
		FRotator newRot = SpringArmComp->RelativeRotation;
		newRot.Pitch = RemoteViewPitch * 360.0f / 255.0f;
		newRot.Pitch = FMath::ClampAngle(newRot.Pitch, -89, 89);

		SpringArmComp->SetRelativeRotation(newRot);
	}

}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookRight", this, &AShooterCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::StartCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AShooterCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	//PlayerInputComponent->BindAction("Jump", IE_Released, this, &AShooterCharacter::StopJumping);

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &AShooterCharacter::StartZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &AShooterCharacter::StopZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShooterCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AShooterCharacter::StopFire);

	PlayerInputComponent->BindAction("ReloadWeapon", IE_Pressed, this, &AShooterCharacter::StartReloadWeapon);
}

FVector AShooterCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

FRotator AShooterCharacter::GetViewRotation() const
{
	if (SpringArmComp && !IsLocallyControlled())
	{
		return SpringArmComp->GetComponentRotation();
	}

	if (Controller != NULL)
	{
		return Controller->GetControlRotation();
	}
	else if (Role < ROLE_Authority)
	{
		// check if being spectated
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = Iterator->Get();
			if (PlayerController && PlayerController->PlayerCameraManager->GetViewTargetPawn() == this)
			{
				return PlayerController->BlendedTargetViewRotation;
			}
		}
	}

	return GetActorRotation();
}

float AShooterCharacter::GetHealth()
{
	if (HealthComp)
	{
		return HealthComp->GetHealth();
	}

	return 0;
}

float AShooterCharacter::GetMaxHealth()
{
	if (HealthComp)
	{
		return HealthComp->GetMaxHealth();
	}

	return 0;
}

void AShooterCharacter::OnHealthChanged(UShooterHealthComponent* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0 && !bDied)
	{
		bDied = true;


		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();

		SetLifeSpan(10.0f);
	}
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterCharacter, CurrentWeapon);
	DOREPLIFETIME(AShooterCharacter, bDied);
	DOREPLIFETIME(AShooterCharacter, bWantsToZoom);
	DOREPLIFETIME(AShooterCharacter, bIsReloadingWeapon);

	//DOREPLIFETIME_CONDITION(AShooterCharacter, CurrentWeapon, COND_OwnerOnly);
}
