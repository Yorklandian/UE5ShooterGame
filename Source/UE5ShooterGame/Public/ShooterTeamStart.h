// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterTeamStart.generated.h"

UCLASS()
class UE5SHOOTERGAME_API AShooterTeamStart : public APlayerStart
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly,Category=Team)
	int32 SpawnTeam;

	UPROPERTY(EditInstanceOnly,Category=Team)
	uint32 bNotForPlayers: 1;

	UPROPERTY(EditInstanceOnly, Category=Team)
	uint32 bNotForBots: 1;

	AShooterTeamStart(const FObjectInitializer& ObjectInitializer);
};
