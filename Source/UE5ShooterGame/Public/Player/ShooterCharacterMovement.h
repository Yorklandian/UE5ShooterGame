// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ShooterCharacterMovement.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UE5SHOOTERGAME_API UShooterCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()
	virtual float GetMaxSpeed() const override;
};
