// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Canvas.h"
#include "GameFramework/Actor.h"
#include "ShooterWeapon.generated.h"

class UAnimMontage;
class AShooterCharacter;
class UAudioComponent;
class UParticleSystem;
class UForceFeedbackEffect;
class USoundCue;
// class UMatineeCameraShake; //TODO:add this plugin

namespace EWeaponState
{
	enum Type
	{
		Idle,
		Firing,
		Reloading,
		Equipping,
	};
}

USTRUCT()
struct FWeaponData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly,Category=Ammo)
	bool bInfiniteAmmo;

	UPROPERTY(EditDefaultsOnly,Category=Ammo)
	bool bInfiniteClip;

	UPROPERTY(EditDefaultsOnly,Category=Ammo)
	int32 MaxAmmo;

	UPROPERTY(EditDefaultsOnly,Category=Ammo)
	int32 AmmoPerClip;

	UPROPERTY(EditDefaultsOnly,Category=Ammo)
	int32 InitialClips;

	UPROPERTY(EditDefaultsOnly,Category=WeaponStat)
	float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly,Category=WeaponStat)
	float NoAnimReloadDuration;

	FWeaponData()
	{
		bInfiniteAmmo = false;
		bInfiniteClip = false;
		MaxAmmo = 100;
		AmmoPerClip = 20;
		InitialClips = 4;
		TimeBetweenShots = 0.2f;
		NoAnimReloadDuration = 1.0f;
	}
};

USTRUCT()
struct FWeaponAnim
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly,Category=Animation)
	UAnimMontage* Pawn1P;

	UPROPERTY(EditDefaultsOnly,Category=Animation)
	UAnimMontage* Pawn3P;

	FWeaponAnim() : Pawn1P(nullptr), Pawn3P(nullptr)
	{
	}
};



UCLASS()
class UE5SHOOTERGAME_API AShooterWeapon : public AActor
{
	GENERATED_BODY()

public:
	AShooterWeapon(const FObjectInitializer& ObjectInitializer);
	
	//Override
	virtual void PostInitializeComponents() override;

	virtual void Destroyed() override;

	/////////////////////////////
	//Ammo
public:
	enum class EAmmoType
	{
		EBullet,
		ERocket,
		EMax,
	};
	
private:
	void GiveAmmo(int AddAmount);

	void UseAmmo();

	virtual EAmmoType GetAmmoType() const
	{
		return EAmmoType::EBullet;
	}

	///////////////////////////////
	//Inventory

	virtual void OnEquip(const AShooterWeapon* LastWeapon);

	virtual void OnEquipFinished();

	virtual void OnUnEquip();

	virtual void OnEnterInventory(AShooterCharacter* NewOwner);

	virtual void OnLeaveInventory();

	bool IsEquipped() const;

	bool IsAttachedToPawn() const;

	///////////////////////////////
	//Input

	/**
	 * @brief local + server
	 */
	virtual void StartFire();

	/**
	 * @brief local + server
	 */
	virtual void StopFire();

	/**
	 * @brief all
	 */
	virtual void StartReload(bool bFromReplication = false);

	/**
	 * @brief local + server
	 */
	virtual void StopReload();

	/**
	 * @brief server
	 */
	virtual void ReloadWeapon();

	UFUNCTION(Reliable, Client)
	void ClientStartReload();

	///////////////////////////////////
	// Control

	bool CanFire() const;

	bool CanReload() const;

	//////////////////////////////////
	// Reading Data
	EWeaponState::Type GetCurrentState() const;

	int32 GetCurrentAmmo() const;

	int32 GetCurrentAmmoInClip() const;

	int32 GetAmmoPerClip() const;

	int32 GetMaxAmmo() const;

	USkeletalMeshComponent* GetWeaponMesh() const;

	UFUNCTION(BlueprintCallable, Category="Game|Weapon")
	AShooterCharacter* GetPawnOwner() const;

	UPROPERTY(EditDefaultsOnly, Category=HUD)
	FCanvasIcon PrimaryIcon;

	UPROPERTY(EditDefaultsOnly, Category=HUD)
	FCanvasIcon SecondaryIcon;

	UPROPERTY(EditDefaultsOnly, Category=HUD)
	FCanvasIcon PrimaryClipIcon;

	UPROPERTY(EditDefaultsOnly, Category=HUD)
	FCanvasIcon SecondaryClipIcon;

	UPROPERTY(EditDefaultsOnly, Category=HUD)
	float AmmoIconsCount;

	UPROPERTY(EditDefaultsOnly, Category=HUD)
	int32 PrimaryClipIconOffset;

	UPROPERTY(EditDefaultsOnly, Category=HUD)
	int32 SecondaryClipIconOffset;

	UPROPERTY(EditDefaultsOnly, Category=HUD)
	FCanvasIcon CrossHair[5];

	/** only use red colored center part of aiming crosshair */
	UPROPERTY(EditDefaultsOnly, Category=HUD)
	bool UseLaserDot;

	UPROPERTY(EditDefaultsOnly, Category=HUD)
	bool UseCustomCrossHair;

	UPROPERTY(EditDefaultsOnly, Category=HUD)
	bool UseCustomAimingCrossHair;

	UPROPERTY(EditDefaultsOnly, Category=HUD)
	bool bHideCrossHairWhileNotAiming;

	UPROPERTY(Transient)
	float TimerIntervalAdjustment;

	UPROPERTY(Config)
	bool bAllowAutomaticWeaponCatchup = true;

	bool HasInfiniteAmmo() const;

	bool HasInfiniteClip() const;

	void SetOwningPawn(AShooterCharacter* ShooterCharacter);

	float GetEquipStartedTime() const;

	float GetEquipDuration() const;

protected:
	UPROPERTY(Transient, ReplicatedUsing=OnRep_MyPawn)
	AShooterCharacter* MyPawn;

	UPROPERTY(EditDefaultsOnly,Category=Config)
	FWeaponData WeaponConfig;

private:
	UPROPERTY(VisibleDefaultsOnly,Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	UPROPERTY(VisibleDefaultsOnly,Category=Mesh)
	USkeletalMeshComponent* Mesh3P;

protected:

	// Sounds and Effects
	UPROPERTY(Transient)
	UAudioComponent* FireAC;

	UPROPERTY(EditDefaultsOnly, Category=Effects)
	FName MuzzleAttachPoint;

	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UParticleSystem* MuzzleFX;

	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;

	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSCSecondary;

	//TODO: ADD plugin
	// UPROPERTY(EditDefaultsOnly, Category=Effects)
	// TSubclassOf<UMatineeCameraShake> FireCameraShake;

	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UForceFeedbackEffect* FireForceFeedback;

	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* FireSound;

	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* FireLoopSound;

	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* FireFinishSound;

	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* OutOfAmmoSound;

	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* ReloadSound;

	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* EquipSound;
	/////////////////////////

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	FWeaponAnim ReloadAnim;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	FWeaponAnim EquipAnim;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	FWeaponAnim FireAnim;

	UPROPERTY(EditDefaultsOnly, Category=Effects)
	uint32 bLoopedMuzzleFX: 1;

	UPROPERTY(EditDefaultsOnly, Category=Sound)
	uint32 bLoopedFireSound: 1;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	uint32 bLoopedFireAnim: 1;
	
	uint32 bPlayingFireAnim: 1;

	uint32 bIsEquipped: 1;

	uint32 bWantsToFire: 1;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_Reload)
	uint32 bPendingReload: 1;

	uint32 bPendingEquip: 1;

	uint32 bRefiring;

	EWeaponState::Type CurrentState;

	float LastFireTime;

	float EquipStartedTime;

	float EquipDuration;

	UPROPERTY(Transient,Replicated)
	int32 CurrentAmmo;

	UPROPERTY(Transient,Replicated)
	int32 CurrentAmmoInClip;

	UPROPERTY(Transient,ReplicatedUsing=OnRep_BurstCounter)
	int32 BurstCounter;
	///////////////////////////////////////
	
	//Time handles
	FTimerHandle TimerHandle_OnEquipFinished;

	FTimerHandle TimerHandle_StopReload;

	FTimerHandle TimerHandle_ReloadWeapon;

	FTimerHandle TimerHandle_HandleFiring;
	///////////////////////////////////////
	
	// Input - server side

	UFUNCTION(Reliable,Server, WithValidation)
	void ServerStartFire();

	UFUNCTION(Reliable,Server, WithValidation)
	void ServerStopFire();

	UFUNCTION(Reliable,Server, WithValidation)
	void ServerStartReload();

	UFUNCTION(Reliable,Server, WithValidation)
	void ServerStopReload();
	///////////////////////////////////////
	///
	// replication and effects

	UFUNCTION()
	void OnRep_MyPawn();

	UFUNCTION()
	void OnRep_BurstCounter();

	UFUNCTION()
	void OnRep_Reload();

	virtual void SimulateWeaponFire();

	virtual void StopSimulatingWeaponFire();
	//////////////////////////////////////

	// Weapon usage
	/**
	 * @brief local
	 */
	virtual void FireWeapon() PURE_VIRTUAL(AShooterWeapon::FireWeapon);

	/**
	 * @brief server
	 *  fire and update ammo
	 */
	UFUNCTION(reliable, server, WithValidation)
	void ServerHandleFiring();

	/**
	 * @brief local + server
	 */
	void HandleReFiring();

	/**
	 * @brief  local + server
	 */
	void HandleFiring();

	/**
	 * @brief local + server
	 */
	virtual void OnBurstStarted();

	/**
	 * @brief local + server
	 */
	virtual void OnBurstFinished();

	void SetWeaponState(EWeaponState::Type NewState);
	
	void DetermineWeaponState();
	///////////////////////////////////////////////////////
	// Inventory

	void AttachMeshToPawn();

	void DetachMeshFromPawn();
	////////////////////////////////
	//Weapon usage helpers
	UAudioComponent* PlayWeaponSound(USoundCue* Sound);

	float PlayWeaponAnimation(const FWeaponAnim& Animation);

	void StopWeaponAnimation(const FWeaponAnim& Animation);

	virtual FVector GetAdjustedAim() const;

	FVector GetCameraAim() const;

	FVector GetCameraDamageStartLocation(const FVector& AimDir) const;

	FVector GetMuzzleLocation() const;

	FVector GetMuzzleDirection() const;

	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;

	FORCEINLINE USkeletalMeshComponent* GetMesh1P() const
	{
		return Mesh1P;
	}

	FORCEINLINE USkeletalMeshComponent* GetMesh3P() const
	{
		return Mesh3P;
	}
	
	
	
};
