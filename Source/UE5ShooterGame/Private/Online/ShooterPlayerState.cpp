// Fill out your copyright notice in the Description page of Project Settings.


#include "Online/ShooterPlayerState.h"

#include "Net/OnlineEngineInterface.h"
#include "Player/ShooterCharacter.h"
#include "Player/ShooterPlayerController.h"
#include "UE5ShooterGame.h"


AShooterPlayerState::AShooterPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	TeamNumber = 0;
	NumKills = 0;
	NumDeaths = 0;
	NumBulletsFired = 0;
	NumRocketsFired = 0;
	bQuitter = false;
}

void AShooterPlayerState::Reset()
{
	Super::Reset();
	NumKills = 0;
	NumDeaths = 0;
	NumBulletsFired = 0;
	NumRocketsFired = 0;
	bQuitter = false;
}

void AShooterPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);
	UpdateTeamColors();
}

void AShooterPlayerState::RegisterPlayerWithSession(bool bWasFromInvite)
{
	if(UOnlineEngineInterface::Get()->DoesSessionExist(GetWorld(),NAME_GameSession))
	{
		Super::RegisterPlayerWithSession(bWasFromInvite);
	}
}

void AShooterPlayerState::UnregisterPlayerWithSession()
{
	if(!IsFromPreviousLevel() && UOnlineEngineInterface::Get()->DoesSessionExist(GetWorld(), NAME_GameSession))
	{
		Super::UnregisterPlayerWithSession();
	}
}

void AShooterPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	AShooterPlayerState* ShooterPlayerState = Cast<AShooterPlayerState>(PlayerState);
	if(ShooterPlayerState)
	{
		ShooterPlayerState->TeamNumber = TeamNumber;
	}
}

void AShooterPlayerState::SetTeamNum(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;
	UpdateTeamColors();
}

void AShooterPlayerState::ScoreKill(AShooterPlayerState* Victim, int32 Points)
{
	NumKills++;
	ScorePoints(Points);
}

void AShooterPlayerState::ScoreDeath(AShooterPlayerState* KilledBy, int32 Points)
{
	NumDeaths++;
	ScorePoints(Points);
}

int32 AShooterPlayerState::GetTeamNum() const
{
	return TeamNumber;
}

int32 AShooterPlayerState::GetKills() const
{
	return NumKills;
}

int32 AShooterPlayerState::GetDeaths() const
{
	return NumDeaths;
}

int32 AShooterPlayerState::GetNumBulletsFired() const
{
	return NumBulletsFired;
}

int32 AShooterPlayerState::GetNumRocketsFired() const
{
	return NumRocketsFired;
}

bool AShooterPlayerState::IsQuitter() const
{
	return bQuitter;
}

FString AShooterPlayerState::GetMatchId() const
{
	return MatchId;
}

FString AShooterPlayerState::GetShorterPlayerName() const
{
	if(GetPlayerName().Len() > MAX_PLAYER_NAME_LENGTH)
	{
		return GetPlayerName().Left(MAX_PLAYER_NAME_LENGTH) + "...";
	}
	return  GetPlayerName();
}

void AShooterPlayerState::InformationAboutKill_Implementation(AShooterPlayerState* KillerPlayerState,
	const UDamageType* KillerDamageType, AShooterPlayerState* KilledPlayerState)
{
	//id can be null for robots
	if(KillerPlayerState->GetUniqueId().IsValid())
	{
		for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();It;++It)
		{
			AShooterPlayerController* TestPC = Cast<AShooterPlayerController>(*It);
			if(TestPC && TestPC->IsLocalController())
			{
				ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(TestPC->Player);
				FUniqueNetIdRepl LocalID = LocalPlayer->GetCachedUniqueNetId();
				if(LocalID.IsValid() && *LocalPlayer->GetCachedUniqueNetId() == *KillerPlayerState->GetUniqueId())
				{
					TestPC->OnKill();
				}
			}
		}
	}
}

void AShooterPlayerState::BroadcastDeath_Implementation(AShooterPlayerState* KillerPlayerState,
	const UDamageType* KillerDamageType, AShooterPlayerState* KilledPlayerState)
{
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();It;++It)
	{
		AShooterPlayerController* TestPC = Cast<AShooterPlayerController>(*It);
		if(TestPC && TestPC->IsLocalController())
		{
			TestPC->OnDeathMessage(KillerPlayerState,this,KillerDamageType);
		}
	}
}

void AShooterPlayerState::OnRep_TeamColor()
{
	UpdateTeamColors();
}

void AShooterPlayerState::AddBulletsFired(int32 NumBullets)
{
	NumBulletsFired += NumBullets;
}

void AShooterPlayerState::AddRocketsFired(int32 NumRockets)
{
	NumRocketsFired += NumRockets;
}

void AShooterPlayerState::SetQuitter(bool bInQuitter)
{
	bQuitter = bInQuitter;
}

void AShooterPlayerState::SetMatchId(const FString& CurrentMatchId)
{
	MatchId = CurrentMatchId;
}

void AShooterPlayerState::UpdateTeamColors()
{
	AController* OwnerController = Cast<AController>(GetOwner());
	if(OwnerController != nullptr)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OwnerController->GetCharacter());
		if(ShooterCharacter != nullptr)
		{
			ShooterCharacter->UpdateTeamColorAllMIDs();
		}
	}
}

void AShooterPlayerState::ScorePoints(int32 Points)
{
	//TODO: add after finishing GameState
}


void AShooterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterPlayerState,TeamNumber);
	DOREPLIFETIME(AShooterPlayerState,NumKills);
	DOREPLIFETIME(AShooterPlayerState,NumDeaths);
	DOREPLIFETIME(AShooterPlayerState,MatchId);
}