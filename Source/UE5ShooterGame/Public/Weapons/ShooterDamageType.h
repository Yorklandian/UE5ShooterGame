// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Canvas.h"
#include "GameFramework/DamageType.h"
#include "ShooterDamageType.generated.h"

/**
 * 
 */
UCLASS()
class UE5SHOOTERGAME_API UShooterDamageType : public UDamageType
{
	GENERATED_BODY()

	/**
	 * @brief icon displayed in death message log when killed with this weapon
	 */
	UPROPERTY(EditDefaultsOnly,Category=HUD)
	FCanvasIcon KillIcon;

	UPROPERTY(EditDefaultsOnly,Category=Effects)
	UForceFeedbackEffect *HitForceFeedback;

	UPROPERTY(EditDefaultsOnly,Category=Effects)
	UForceFeedbackEffect *KilledForceFeedback;
};
