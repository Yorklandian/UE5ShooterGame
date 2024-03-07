// Fill out your copyright notice in the Description page of Project Settings.


#include "UE5ShooterGame/Public/Player/ShooterCharacter.h"



// Sets default values
AShooterCharacter::AShooterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AShooterCharacter::UpdateTeamColorAllMIDs()
{
}

bool AShooterCharacter::CanFire() const
{
	return false;
}

bool AShooterCharacter::CanReload() const
{
	return false;
}

AShooterWeapon* AShooterCharacter::GetWeapon() const
{
	return nullptr;
}

bool AShooterCharacter::IsFirstPerson() const
{
	return false;
}

FName AShooterCharacter::GetWeaponAttachPoint() const
{
	return {};
}

USkeletalMeshComponent* AShooterCharacter::GetSpecifcPawnMesh(bool WantFirstPerson) const
{
	return nullptr;
}

USkeletalMeshComponent* AShooterCharacter::GetPawnMesh() const
{
	return nullptr;
}

bool AShooterCharacter::IsTargeting() const
{
	return false;
}



