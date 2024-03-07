// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapons/ShooterWeapon.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class UE5SHOOTERGAME_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void UpdateTeamColorAllMIDs();

	bool CanFire() const;

	bool CanReload() const;

	AShooterWeapon* GetWeapon() const;

	UFUNCTION(BlueprintCallable, Category = Mesh)
	virtual bool IsFirstPerson() const;

	FName GetWeaponAttachPoint() const;

	/*
	* Get either first or third person mesh.
	*
	* @param	WantFirstPerson		If true returns the first peron mesh, else returns the third
	*/
	USkeletalMeshComponent* GetSpecifcPawnMesh(bool WantFirstPerson) const;

	/** get mesh component */
	USkeletalMeshComponent* GetPawnMesh() const;

	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	bool IsTargeting() const;
};
