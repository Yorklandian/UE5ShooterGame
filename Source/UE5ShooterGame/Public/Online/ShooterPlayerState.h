// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ShooterPlayerState.generated.h"

UCLASS()
class UE5SHOOTERGAME_API AShooterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	AShooterPlayerState(const FObjectInitializer& ObjectInitializer);
	
	//Begin overriding APlayerState interfaces
	virtual void Reset() override;

	virtual void ClientInitialize(AController* C) override;

	virtual void RegisterPlayerWithSession(bool bWasFromInvite) override;

	virtual void UnregisterPlayerWithSession() override;

	virtual void CopyProperties(APlayerState* PlayerState) override;

	//End APlayerState interfaces

	void SetTeamNum(int32 NewTeamNumber);
	
	void ScoreKill(AShooterPlayerState* Victim, int32 Points);

	void ScoreDeath(AShooterPlayerState* KilledBy, int32 Points);

	int32 GetTeamNum() const;

	int32 GetKills() const;

	int32 GetDeaths() const;

	int32 GetNumBulletsFired() const;

	int32 GetNumRocketsFired() const;

	bool IsQuitter() const;

	FString GetMatchId() const;

	FString GetShorterPlayerName() const;

	UFUNCTION(Reliable, Client)
	void InformationAboutKill(class AShooterPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AShooterPlayerState* KilledPlayerState);

	UFUNCTION(Reliable,NetMulticast)
	void BroadcastDeath(class AShooterPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AShooterPlayerState* KilledPlayerState);

	UFUNCTION()
	void OnRep_TeamColor();

	void AddBulletsFired(int32 NumBullets);

	void AddRocketsFired(int32 NumRockets);

	void SetQuitter(bool bInQuitter);

	void SetMatchId(const FString& CurrentMatchId);

	

protected:
	void UpdateTeamColors();

	UPROPERTY(Transient,ReplicatedUsing=OnRep_TeamColor);
	int32 TeamNumber;

	UPROPERTY(Transient, Replicated)
	int32 NumKills;

	UPROPERTY(Transient, Replicated)
	int32 NumDeaths;

	UPROPERTY(Transient, Replicated)
	int32 NumBulletsFired;

	UPROPERTY()
	int32 NumRocketsFired;

	UPROPERTY()
	uint8 bQuitter: 1;

	UPROPERTY(Replicated)
	FString MatchId;

	void ScorePoints(int32 Points);
};
