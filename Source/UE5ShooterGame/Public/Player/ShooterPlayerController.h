// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

UCLASS()
class UE5SHOOTERGAME_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AShooterPlayerController();

	void OnKill();
	void OnDeathMessage(class AShooterPlayerState* KillerPlayerState, class AShooterPlayerState* KilledPlayerState, const UDamageType* killerDamageType);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** get infinite ammo cheat */
	bool HasInfiniteAmmo() const;

	/** get infinite clip cheat */
	bool HasInfiniteClip() const;
};
