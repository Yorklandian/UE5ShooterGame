// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ShooterPlayerController.h"


// Sets default values
AShooterPlayerController::AShooterPlayerController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AShooterPlayerController::OnKill()
{
}

void AShooterPlayerController::OnDeathMessage(AShooterPlayerState* KillerPlayerState,
	AShooterPlayerState* KilledPlayerState, const UDamageType* killerDamageType)
{
}

// Called when the game starts or when spawned
void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AShooterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AShooterPlayerController::HasInfiniteAmmo() const
{
	return false;
}

bool AShooterPlayerController::HasInfiniteClip() const
{
	return false;
}

