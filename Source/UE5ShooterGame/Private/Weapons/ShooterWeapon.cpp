// Fill out your copyright notice in the Description page of Project Settings.


#include "..\..\Public\Weapons\ShooterWeapon.h"

// #include "Bots/ShooterAIController.h"
#include "Player/ShooterCharacter.h"
#include "Player/ShooterPlayerController.h"
#include "UE5ShooterGame/UE5ShooterGame.h"


// Sets default values


AShooterWeapon::AShooterWeapon(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this,TEXT("WeaponMesh1P"));
	Mesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh1P->bReceivesDecals = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh1P->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = Mesh1P;

	Mesh3P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this,TEXT("WeaponMesh3P"));
	Mesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->CastShadow = true;
	Mesh3P->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3P->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh3P->SetCollisionResponseToChannel(COLLISION_WEAPON,ECR_Block);
	Mesh3P->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
	Mesh3P->SetCollisionResponseToChannel(COLLISION_PROJECTILE,ECR_Block);
	Mesh3P->SetupAttachment(Mesh1P);

	bLoopedMuzzleFX = false;
	bLoopedFireAnim = false;
	bPlayingFireAnim = false;
	bIsEquipped = false;
	bWantsToFire = false;
	bPendingReload = false;
	bPendingEquip = false;
	CurrentState = EWeaponState::Idle;

	CurrentAmmo = 0;
	CurrentAmmoInClip = 0;
	BurstCounter = 0;
	LastFireTime = 0.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
}

void AShooterWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(WeaponConfig.InitialClips > 0)
	{
		CurrentAmmoInClip = WeaponConfig.AmmoPerClip;
		CurrentAmmo = WeaponConfig.AmmoPerClip * WeaponConfig.InitialClips;
	}
}

void AShooterWeapon::Destroyed()
{
	Super::Destroyed();
	StopSimulatingWeaponFire();
}

void AShooterWeapon::GiveAmmo(int AddAmount)
{
	const int32 MissingAmmo = FMath::Max(0,WeaponConfig.MaxAmmo-CurrentAmmo);
	AddAmount = FMath::Min(AddAmount,MissingAmmo);
	CurrentAmmo += AddAmount;

	// TODO: Add AI controller later
	// AShooterAIController* BotAI = MyPawn ? Cast<AShooterAIController>(MyPawn->GetController()) : nullptr;
	// if (BotAI)
	// {
	// 	BotAI->CheckAmmo(this);
	// }
	
	// start reload if clip was empty
	if (GetCurrentAmmoInClip() <= 0 &&
		CanReload() &&
		MyPawn && (MyPawn->GetWeapon() == this))
	{
		ClientStartReload();
	}
}

void AShooterWeapon::UseAmmo()
{
	if (!HasInfiniteAmmo())
	{
		CurrentAmmoInClip--;
	}

	if (!HasInfiniteAmmo() && !HasInfiniteClip())
	{
		CurrentAmmo--;
	}

	//TODO: uncomment after finishing AI
	/*AShooterAIController* BotAI = MyPawn ? Cast<AShooterAIController>(MyPawn->GetController()) : NULL;	
	AShooterPlayerController* PlayerController = MyPawn ? Cast<AShooterPlayerController>(MyPawn->GetController()) : NULL;
	if (BotAI)
	{
		BotAI->CheckAmmo(this);
	}
	else if(PlayerController)
	{
		if (AShooterPlayerState* PlayerState = Cast<AShooterPlayerState>(PlayerController->PlayerState))
		{
			switch (GetAmmoType())
			{
			case EAmmoType::ERocket:
				PlayerState->AddRocketsFired(1);
				break;
			case EAmmoType::EBullet:
			default:
				PlayerState->AddBulletsFired(1);
				break;
			}
		}
	}*/
}

void AShooterWeapon::OnEquip(const AShooterWeapon* LastWeapon)
{
	AttachMeshToPawn();

	bPendingEquip = true;
	DetermineWeaponState();
	//only play animation if has a lastWeapon
	if(LastWeapon)
	{
		float Duration = PlayWeaponAnimation(EquipAnim);
		if(Duration <= 0.0f)
		{
			Duration = 0.5f;
		}
		EquipStartedTime = GetWorld()->GetTimeSeconds();
		EquipDuration = Duration;

		GetWorldTimerManager().SetTimer(TimerHandle_OnEquipFinished,this, &AShooterWeapon::OnEquipFinished,Duration,false);
		
	}
	else
	{
		OnEquipFinished();
	}

	//Equip Sound is only played locally
	if(MyPawn && MyPawn->IsLocallyControlled())
	{
		PlayWeaponSound(EquipSound);
	}
	
}

void AShooterWeapon::OnEquipFinished()
{
	AttachMeshToPawn();

	bIsEquipped = true;
	bPendingEquip = false;

	DetermineWeaponState();

	if(MyPawn)
	{
		if(MyPawn->IsLocallyControlled() && CurrentAmmoInClip <= 0 && CanReload())
		{
			StartReload();
		}
	}
}

void AShooterWeapon::OnUnEquip()
{
	DetachMeshFromPawn();
	bIsEquipped = false;
	StopFire();

	//stop reload anim if is reloading
	if(bPendingReload)
	{
		StopWeaponAnimation(ReloadAnim);
		bPendingReload = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_StopReload);
		GetWorldTimerManager().ClearTimer(TimerHandle_ReloadWeapon);
	}

	//if is equipping, stop equip
	if(bPendingEquip)
	{
		StopWeaponAnimation(EquipAnim);
		bPendingEquip = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_OnEquipFinished);
	}

	//TODO: Add delegate broadcast after finishing AShooterCharacter
}

void AShooterWeapon::OnEnterInventory(AShooterCharacter* NewOwner)
{
	SetOwningPawn(NewOwner);
}

void AShooterWeapon::OnLeaveInventory()
{
	if(IsAttachedToPawn())
	{
		OnUnEquip();
	}

	if(GetLocalRole() == ROLE_Authority)
	{
		SetOwningPawn(nullptr);
	}
}

bool AShooterWeapon::IsEquipped() const
{
	return bIsEquipped;
}

bool AShooterWeapon::IsAttachedToPawn() const
{
	return bIsEquipped || bPendingEquip;
}

void AShooterWeapon::StartFire()
{
	if(GetLocalRole() < ROLE_Authority)
	{
		ServerStartFire();
	}

	if(!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

void AShooterWeapon::StopFire()
{
	if((GetLocalRole() < ROLE_Authority) && MyPawn && MyPawn->IsLocallyControlled())
	{
		ServerStopFire();
	}

	if(bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

void AShooterWeapon::StartReload(bool bFromReplication)
{
	if(!bFromReplication && GetLocalRole() <ROLE_Authority)
	{
		ServerStartReload();
	}

	if(bFromReplication || CanReload())
	{
		bPendingReload = true;
		DetermineWeaponState();
		float AnimDuration = PlayWeaponAnimation(ReloadAnim);
		if(AnimDuration <= 0.0f)
		{
			AnimDuration = WeaponConfig.NoAnimReloadDuration;
		}

		GetWorldTimerManager().SetTimer(TimerHandle_StopReload,this,&AShooterWeapon::StopReload,AnimDuration,false);
		if(GetLocalRole() == ROLE_Authority)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &AShooterWeapon::ReloadWeapon, FMath::Max(0.1f, AnimDuration - 0.1f), false);
		}

		if(MyPawn && MyPawn->IsLocallyControlled())
		{
			PlayWeaponSound(ReloadSound);
		}
	}
}

void AShooterWeapon::StopReload()
{
	if(CurrentState == EWeaponState::Reloading)
	{
		bPendingReload = false;
		DetermineWeaponState();
		StopWeaponAnimation(ReloadAnim);
	}
}

void AShooterWeapon::ReloadWeapon()
{
	int32 ClipDelta = FMath::Min(WeaponConfig.AmmoPerClip-CurrentAmmoInClip, CurrentAmmo-CurrentAmmoInClip);
	if(HasInfiniteClip())
	{
		ClipDelta = WeaponConfig.AmmoPerClip- CurrentAmmoInClip;
	}

	if(ClipDelta > 0)
	{
		CurrentAmmoInClip += ClipDelta;
	}

	if(HasInfiniteClip())
	{
		CurrentAmmo = FMath::Max(CurrentAmmoInClip, CurrentAmmo);
	}
}

void AShooterWeapon::ClientStartReload_Implementation()
{
	StartReload();
}

bool AShooterWeapon::CanFire() const
{
	bool bCanFire = MyPawn && MyPawn->CanFire();
	bool bStateOKToFire = ( ( CurrentState ==  EWeaponState::Idle ) || ( CurrentState == EWeaponState::Firing) );	
	return (( bCanFire == true ) && ( bStateOKToFire == true ) && ( bPendingReload == false ));
}

bool AShooterWeapon::CanReload() const
{
	bool bCanReload = (!MyPawn || MyPawn->CanReload());
	bool bGotAmmo = ( CurrentAmmoInClip < WeaponConfig.AmmoPerClip) && (CurrentAmmo - CurrentAmmoInClip > 0 || HasInfiniteClip());
	bool bStateOKToReload = ( ( CurrentState ==  EWeaponState::Idle ) || ( CurrentState == EWeaponState::Firing) );
	return ( ( bCanReload == true ) && ( bGotAmmo == true ) && ( bStateOKToReload == true) );	
}

EWeaponState::Type AShooterWeapon::GetCurrentState() const
{
	return CurrentState;
}

int32 AShooterWeapon::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

int32 AShooterWeapon::GetCurrentAmmoInClip() const
{
	return CurrentAmmoInClip;
}

int32 AShooterWeapon::GetAmmoPerClip() const
{
	return WeaponConfig.AmmoPerClip;
}

int32 AShooterWeapon::GetMaxAmmo() const
{
	return WeaponConfig.MaxAmmo;
}

USkeletalMeshComponent* AShooterWeapon::GetWeaponMesh() const
{
	return (MyPawn != nullptr && MyPawn->IsFirstPerson()) ? Mesh1P : Mesh3P;
}

AShooterCharacter* AShooterWeapon::GetPawnOwner() const
{
	return MyPawn;
}

bool AShooterWeapon::HasInfiniteAmmo() const
{
	const AShooterPlayerController* MyPC = (MyPawn != nullptr) ? Cast<const AShooterPlayerController>(MyPawn->Controller) : nullptr;
	return WeaponConfig.bInfiniteAmmo || (MyPC && MyPC->HasInfiniteAmmo());
}

bool AShooterWeapon::HasInfiniteClip() const
{
	const AShooterPlayerController* MyPC = (MyPawn != nullptr) ? Cast<const AShooterPlayerController>(MyPawn->Controller) : nullptr;
	return WeaponConfig.bInfiniteClip || (MyPC && MyPC->HasInfiniteClip());
}

void AShooterWeapon::SetOwningPawn(AShooterCharacter* ShooterCharacter)
{
	if (MyPawn != ShooterCharacter)
	{
		SetInstigator(ShooterCharacter);
		MyPawn = ShooterCharacter;
		// net owner for RPC calls
		SetOwner(ShooterCharacter);
	}	
}

float AShooterWeapon::GetEquipStartedTime() const
{
	return EquipStartedTime;
}

float AShooterWeapon::GetEquipDuration() const
{
	return EquipDuration;
}

void AShooterWeapon::ServerStartFire_Implementation()
{
	StartFire();
}

bool AShooterWeapon::ServerStartFire_Validate()
{
	return true;
}

void AShooterWeapon::ServerStopFire_Implementation()
{
	StopFire();
}

bool AShooterWeapon::ServerStopFire_Validate()
{
	return true;
}

void AShooterWeapon::ServerStartReload_Implementation()
{
	StartReload();
}

bool AShooterWeapon::ServerStartReload_Validate()
{
	return true;
}

void AShooterWeapon::ServerStopReload_Implementation()
{
	StopReload();
}

bool AShooterWeapon::ServerStopReload_Validate()
{
	return true;
}

void AShooterWeapon::OnRep_MyPawn()
{
	if (MyPawn)
	{
		OnEnterInventory(MyPawn);
	}
	else
	{
		OnLeaveInventory();
	}
}

void AShooterWeapon::OnRep_BurstCounter()
{
	if (BurstCounter > 0)
	{
		SimulateWeaponFire();
	}
	else
	{
		StopSimulatingWeaponFire();
	}
}

void AShooterWeapon::OnRep_Reload()
{
	if (bPendingReload)
	{
		StartReload(true);
	}
	else
	{
		StopReload();
	}
}

void AShooterWeapon::SimulateWeaponFire()
{
	//TODO: uncomment after finished
	// if (GetLocalRole() == ROLE_Authority && CurrentState != EWeaponState::Firing)
	// {
	// 	return;
	// }
	//
	// if (MuzzleFX)
	// {
	// 	USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
	// 	if (!bLoopedMuzzleFX || MuzzlePSC == NULL)
	// 	{
	// 		// Split screen requires we create 2 effects. One that we see and one that the other player sees.
	// 		if( (MyPawn != NULL ) && ( MyPawn->IsLocallyControlled() == true ) )
	// 		{
	// 			AController* PlayerCon = MyPawn->GetController();				
	// 			if( PlayerCon != NULL )
	// 			{
	// 				Mesh1P->GetSocketLocation(MuzzleAttachPoint);
	// 				MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh1P, MuzzleAttachPoint);
	// 				MuzzlePSC->bOwnerNoSee = false;
	// 				MuzzlePSC->bOnlyOwnerSee = true;
	//
	// 				Mesh3P->GetSocketLocation(MuzzleAttachPoint);
	// 				MuzzlePSCSecondary = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh3P, MuzzleAttachPoint);
	// 				MuzzlePSCSecondary->bOwnerNoSee = true;
	// 				MuzzlePSCSecondary->bOnlyOwnerSee = false;				
	// 			}				
	// 		}
	// 		else
	// 		{
	// 			MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, UseWeaponMesh, MuzzleAttachPoint);
	// 		}
	// 	}
	// }
	//
	// if (!bLoopedFireAnim || !bPlayingFireAnim)
	// {
	// 	PlayWeaponAnimation(FireAnim);
	// 	bPlayingFireAnim = true;
	// }
	//
	// if (bLoopedFireSound)
	// {
	// 	if (FireAC == NULL)
	// 	{
	// 		FireAC = PlayWeaponSound(FireLoopSound);
	// 	}
	// }
	// else
	// {
	// 	PlayWeaponSound(FireSound);
	// }
	//
	// AShooterPlayerController* PC = (MyPawn != NULL) ? Cast<AShooterPlayerController>(MyPawn->Controller) : NULL;
	// if (PC != NULL && PC->IsLocalController())
	// {
	// 	if (FireCameraShake != NULL)
	// 	{
	// 		PC->ClientStartCameraShake(FireCameraShake, 1);
	// 	}
	// 	if (FireForceFeedback != NULL && PC->IsVibrationEnabled())
	// 	{
	// 		FForceFeedbackParameters FFParams;
	// 		FFParams.Tag = "Weapon";
	// 		PC->ClientPlayForceFeedback(FireForceFeedback, FFParams);
	// 	}
	// }
}

void AShooterWeapon::StopSimulatingWeaponFire()
{
	if (bLoopedMuzzleFX )
	{
		if( MuzzlePSC != nullptr )
		{
			MuzzlePSC->DeactivateSystem();
			MuzzlePSC = nullptr;
		}
		if( MuzzlePSCSecondary != nullptr )
		{
			MuzzlePSCSecondary->DeactivateSystem();
			MuzzlePSCSecondary = nullptr;
		}
	}

	if (bLoopedFireAnim && bPlayingFireAnim)
	{
		StopWeaponAnimation(FireAnim);
		bPlayingFireAnim = false;
	}

	if (FireAC)
	{
		FireAC->FadeOut(0.1f, 0.0f);
		FireAC = nullptr;

		PlayWeaponSound(FireFinishSound);
	}
}


void AShooterWeapon::ServerHandleFiring_Implementation()
{
	const bool bShouldUpdateAmmo = (CurrentAmmoInClip > 0 && CanFire());

	HandleFiring();

	if (bShouldUpdateAmmo)
	{
		// update ammo
		UseAmmo();

		// update firing FX on remote clients
		BurstCounter++;
	}
}

bool AShooterWeapon::ServerHandleFiring_Validate()
{
	return true;
}

void AShooterWeapon::HandleReFiring()
{
	// Update TimerIntervalAdjustment
	UWorld* MyWorld = GetWorld();

	float SlackTimeThisFrame = FMath::Max(0.0f, (MyWorld->TimeSeconds - LastFireTime) - WeaponConfig.TimeBetweenShots);

	if (bAllowAutomaticWeaponCatchup)
	{
		TimerIntervalAdjustment -= SlackTimeThisFrame;
	}

	HandleFiring();
}

void AShooterWeapon::HandleFiring()
{
	if ((CurrentAmmoInClip > 0 || HasInfiniteClip() || HasInfiniteAmmo()) && CanFire())
	{
		if (GetNetMode() != NM_DedicatedServer)
		{
			SimulateWeaponFire();
		}

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			FireWeapon();

			UseAmmo();
			
			// update firing FX on remote clients if function was called on server
			BurstCounter++;
		}
	}
	else if (CanReload())
	{
		StartReload();
	}
	else if (MyPawn && MyPawn->IsLocallyControlled())
	{
		if (GetCurrentAmmo() == 0 && !bRefiring)
		{
			PlayWeaponSound(OutOfAmmoSound);
			AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(MyPawn->Controller);
			//TODO: uncomment after finish
			// AShooterHUD* MyHUD = MyPC ? Cast<AShooterHUD>(MyPC->GetHUD()) : NULL;
			// if (MyHUD)
			// {
			// 	MyHUD->NotifyOutOfAmmo();
			// }
		}
		
		// stop weapon fire FX, but stay in Firing state
		if (BurstCounter > 0)
		{
			OnBurstFinished();
		}
	}
	else
	{
		OnBurstFinished();
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		// local client will notify server
		if (GetLocalRole() < ROLE_Authority)
		{
			ServerHandleFiring();
		}

		// reload after firing last round
		if (CurrentAmmoInClip <= 0 && CanReload())
		{
			StartReload();
		}

		// setup refire timer
		bRefiring = (CurrentState == EWeaponState::Firing && WeaponConfig.TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AShooterWeapon::HandleReFiring, FMath::Max<float>(WeaponConfig.TimeBetweenShots + TimerIntervalAdjustment, SMALL_NUMBER), false);
			TimerIntervalAdjustment = 0.f;
		}
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}

void AShooterWeapon::OnBurstStarted()
{
	// start firing, can be delayed to satisfy TimeBetweenShots
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && WeaponConfig.TimeBetweenShots > 0.0f &&
		LastFireTime + WeaponConfig.TimeBetweenShots > GameTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AShooterWeapon::HandleFiring, LastFireTime + WeaponConfig.TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void AShooterWeapon::OnBurstFinished()
{
	// stop firing FX on remote clients
	BurstCounter = 0;

	// stop firing FX locally, unless it's a dedicated server
	//if (GetNetMode() != NM_DedicatedServer)
	//{
	StopSimulatingWeaponFire();
	//}
	
	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
	bRefiring = false;

	// reset firing interval adjustment
	TimerIntervalAdjustment = 0.0f;
}

void AShooterWeapon::SetWeaponState(EWeaponState::Type NewState)
{
	const EWeaponState::Type PrevState = CurrentState;

	if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
	{
		OnBurstStarted();
	}
}

void AShooterWeapon::DetermineWeaponState()
{
	EWeaponState::Type NewState = EWeaponState::Idle;

	if (bIsEquipped)
	{
		if( bPendingReload  )
		{
			if( CanReload() == false )
			{
				NewState = CurrentState;
			}
			else
			{
				NewState = EWeaponState::Reloading;
			}
		}		
		else if ( (bPendingReload == false ) && ( bWantsToFire == true ) && ( CanFire() == true ))
		{
			NewState = EWeaponState::Firing;
		}
	}
	else if (bPendingEquip)
	{
		NewState = EWeaponState::Equipping;
	}

	SetWeaponState(NewState);
}

void AShooterWeapon::AttachMeshToPawn()
{
	if (MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName AttachPoint = MyPawn->GetWeaponAttachPoint();
		if( MyPawn->IsLocallyControlled() == true )
		{
			USkeletalMeshComponent* PawnMesh1p = MyPawn->GetSpecifcPawnMesh(true);
			USkeletalMeshComponent* PawnMesh3p = MyPawn->GetSpecifcPawnMesh(false);
			Mesh1P->SetHiddenInGame( false );
			Mesh3P->SetHiddenInGame( false );
			Mesh1P->AttachToComponent(PawnMesh1p, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			Mesh3P->AttachToComponent(PawnMesh3p, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
		}
		else
		{
			USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
			USkeletalMeshComponent* UsePawnMesh = MyPawn->GetPawnMesh();
			UseWeaponMesh->AttachToComponent(UsePawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			UseWeaponMesh->SetHiddenInGame( false );
		}
	}
}

void AShooterWeapon::DetachMeshFromPawn()
{
	Mesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh1P->SetHiddenInGame(true);

	Mesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh3P->SetHiddenInGame(true);
}

UAudioComponent* AShooterWeapon::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AC = nullptr;
	if (Sound && MyPawn)
	{
		AC = UGameplayStatics::SpawnSoundAttached(Sound, MyPawn->GetRootComponent());
	}

	return AC;
}

float AShooterWeapon::PlayWeaponAnimation(const FWeaponAnim& Animation)
{
	float Duration = 0.0f;
	if (MyPawn)
	{
		UAnimMontage* UseAnim = MyPawn->IsFirstPerson() ? Animation.Pawn1P : Animation.Pawn3P;
		if (UseAnim)
		{
			Duration = MyPawn->PlayAnimMontage(UseAnim);
		}
	}

	return Duration;
}

void AShooterWeapon::StopWeaponAnimation(const FWeaponAnim& Animation)
{
	if (MyPawn)
	{
		UAnimMontage* UseAnim = MyPawn->IsFirstPerson() ? Animation.Pawn1P : Animation.Pawn3P;
		if (UseAnim)
		{
			MyPawn->StopAnimMontage(UseAnim);
		}
	}
}

FVector AShooterWeapon::GetAdjustedAim() const
{
	AShooterPlayerController* const PlayerController = GetInstigatorController<AShooterPlayerController>();
	FVector FinalAim = FVector::ZeroVector;
	// If we have a player controller use it for the aim
	if (PlayerController)
	{
		FVector CamLoc;
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}
	else if (GetInstigator())
	{
		//TODO: uncomment after finish
		// // Now see if we have an AI controller - we will want to get the aim from there if we do
		// AShooterAIController* AIController = MyPawn ? Cast<AShooterAIController>(MyPawn->Controller) : NULL;
		// if(AIController != NULL )
		// {
		// 	FinalAim = AIController->GetControlRotation().Vector();
		// }
		// else
		// {			
		// 	FinalAim = GetInstigator()->GetBaseAimRotation().Vector();
		// }
	}

	return FinalAim;
}

FVector AShooterWeapon::GetCameraAim() const
{
	AShooterPlayerController* const PlayerController = GetInstigatorController<AShooterPlayerController>();
	FVector FinalAim = FVector::ZeroVector;

	if (PlayerController)
	{
		FVector CamLoc;
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}
	else if (GetInstigator())
	{
		FinalAim = GetInstigator()->GetBaseAimRotation().Vector();		
	}

	return FinalAim;
}

FVector AShooterWeapon::GetCameraDamageStartLocation(const FVector& AimDir) const
{
	//TODO: uncomment after AI controller
	// AShooterPlayerController* PC = MyPawn ? Cast<AShooterPlayerController>(MyPawn->Controller) : NULL;
	// AShooterAIController* AIPC = MyPawn ? Cast<AShooterAIController>(MyPawn->Controller) : NULL;
	// FVector OutStartTrace = FVector::ZeroVector;
	//
	// if (PC)
	// {
	// 	// use player's camera
	// 	FRotator UnusedRot;
	// 	PC->GetPlayerViewPoint(OutStartTrace, UnusedRot);
	//
	// 	// Adjust trace so there is nothing blocking the ray between the camera and the pawn, and calculate distance from adjusted start
	// 	OutStartTrace = OutStartTrace + AimDir * ((GetInstigator()->GetActorLocation() - OutStartTrace) | AimDir);
	// }
	// else if (AIPC)
	// {
	// 	OutStartTrace = GetMuzzleLocation();
	// }
	//
	// return OutStartTrace;
	return {};
}

FVector AShooterWeapon::GetMuzzleLocation() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketLocation(MuzzleAttachPoint);
}

FVector AShooterWeapon::GetMuzzleDirection() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketRotation(MuzzleAttachPoint).Vector();
}

FHitResult AShooterWeapon::WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const
{
	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceFrom,TraceTo, COLLISION_WEAPON, TraceParams);

	return Hit;
}

void AShooterWeapon::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	DOREPLIFETIME( AShooterWeapon, MyPawn );

	DOREPLIFETIME_CONDITION( AShooterWeapon, CurrentAmmo,		COND_OwnerOnly );
	DOREPLIFETIME_CONDITION( AShooterWeapon, CurrentAmmoInClip, COND_OwnerOnly );

	DOREPLIFETIME_CONDITION( AShooterWeapon, BurstCounter,		COND_SkipOwner );
	DOREPLIFETIME_CONDITION( AShooterWeapon, bPendingReload,	COND_SkipOwner );
}
