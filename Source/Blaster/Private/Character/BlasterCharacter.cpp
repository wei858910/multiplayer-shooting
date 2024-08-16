// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"
#include "BlasterComponents/CombatComponent.h"
#include "BlasterComponents/BuffComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Blaster.h"
#include "PlayerController/BlasterPlayerController.h"
#include "GameMode/BlasterGameMode.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "PlayerState/BlasterPlayerState.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "BlasterComponents/LagCompensationComponent_Plus.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GameState/BlasterGameState.h"

#define LAGCOMPENSATIONCOMPONENT_REGISTER(ComponentName) \
	ComponentName = CreateDefaultSubobject<UBoxComponent>(TEXT(#ComponentName)); \
	ComponentName->SetupAttachment(GetMesh(), FName(#ComponentName)); \
	ComponentName->SetCollisionObjectType(ECC_HitBox);	\
	ComponentName->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);	\
	ComponentName->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);	\
	ComponentName->SetCollisionEnabled(ECollisionEnabled::NoCollision); \
	HitCollisionBoxes.Add(FName(#ComponentName), ComponentName)
	

#define LAGCOMPENSATIONCOMPONENT_SOCKETNAME_REGISTER(ComponentName, SocketName) \
	ComponentName = CreateDefaultSubobject<UBoxComponent>(TEXT(#ComponentName)); \
	ComponentName->SetupAttachment(GetMesh(), FName(#SocketName)); \
	ComponentName->SetCollisionObjectType(ECC_HitBox);	\
	ComponentName->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);	\
	ComponentName->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);	\
	ComponentName->SetCollisionEnabled(ECollisionEnabled::NoCollision); \
	HitCollisionBoxes.Add(FName(#ComponentName), ComponentName)

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	bIsCrouched = true;

	//总是生成
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SetRelativeLocation(FVector(0, 0, BaseEyeHeight));
	CameraBoom->bEnableCameraLag = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);
	OverheadWidget->SetDrawAtDesiredSize(true);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->SetCrouchedHalfHeight(60.f);
	GetCharacterMovement()->MaxWalkSpeedCrouched = 100.0f;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 850.0f, 0.0f);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	TurningInterpSpeed = 10.0f;

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetVisibility(false);

	/** 
	* Hit boxes used for server-side rewind
	*/
	LAGCOMPENSATIONCOMPONENT_REGISTER(head);
	LAGCOMPENSATIONCOMPONENT_REGISTER(pelvis);
	LAGCOMPENSATIONCOMPONENT_REGISTER(spine_02);
	LAGCOMPENSATIONCOMPONENT_REGISTER(spine_03);
	LAGCOMPENSATIONCOMPONENT_REGISTER(upperarm_l);
	LAGCOMPENSATIONCOMPONENT_REGISTER(upperarm_r);
	LAGCOMPENSATIONCOMPONENT_REGISTER(lowerarm_l);
	LAGCOMPENSATIONCOMPONENT_REGISTER(lowerarm_r);
	LAGCOMPENSATIONCOMPONENT_REGISTER(hand_l);
	LAGCOMPENSATIONCOMPONENT_REGISTER(hand_r);
	LAGCOMPENSATIONCOMPONENT_REGISTER(backpack);
	LAGCOMPENSATIONCOMPONENT_SOCKETNAME_REGISTER(blanket, backpack);
	LAGCOMPENSATIONCOMPONENT_REGISTER(thigh_l);
	LAGCOMPENSATIONCOMPONENT_REGISTER(thigh_r);
	LAGCOMPENSATIONCOMPONENT_REGISTER(calf_l);
	LAGCOMPENSATIONCOMPONENT_REGISTER(calf_r);
	LAGCOMPENSATIONCOMPONENT_REGISTER(foot_l);
	LAGCOMPENSATIONCOMPONENT_REGISTER(foot_r);

	// 延迟补偿组件
	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent_Plus>(TEXT("LagCompensationComponent"));
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, TurningInPlace, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, AO_Yaw, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, AO_Pitch, COND_SimulatedOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shield);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}

	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}

	if (LagCompensationComponent)
	{
		LagCompensationComponent->BlasterCharacter = this;
		LagCompensationComponent->BlasterController = Cast<ABlasterPlayerController>(Controller);
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	//SpawnDefaultWeapon(); // 这种事情在Controller的OnPosses函数中做，因为这里一定有Controller变量有值
	UpdateHUDAmmo();

	BlasterController = Cast<ABlasterPlayerController>(Controller);

	UpdateHUDHealth();
	UpdateHUDShield();
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);

	HideCameraIfCharacterClose();

	PollInit();
}

void ABlasterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.0f);
			BlasterPlayerState->AddToDefeat(0);
			SetTeamColor(BlasterPlayerState->GetTeam());

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			if (BlasterGameState && BlasterGameState->TopScoringPlayer.Contains(BlasterPlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;
	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName, FireWeaponMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr && Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName;
		SectionName = FName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName(TEXT("Rifle"));
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName(TEXT("RocketLauncher"));
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName(TEXT("Pistol"));
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName(TEXT("Shotgun"));
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName(TEXT("SniperRifle"));
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName(TEXT("GrenadeLauncher"));
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

//废弃了
void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlasterCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

void ABlasterCharacter::Elimination(bool bPlayerLeftGame)
{
	if (!HasAuthority()) return;
	DropOrDestroyWeapons();
	MulticastElimination(bPlayerLeftGame);
}

void ABlasterCharacter::MulticastElimination_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;

	if (BlasterController)
	{
		BlasterController->SetHUDWeaponAmmo(0, 0);
	}
	bElimmed = true;

	// 视角设置回正常
	if (Combat)
	{
		Combat->SetAiming(false);
	}

	PlayElimMontage();//可以不用

	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);

		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	//Disable Collision
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (BlasterController)
	{
		DisableInput(BlasterController);
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FVector ElimBotLocation(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
	if (ElimBotEffect)
	{
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotLocation,
			GetActorRotation()
		);
	}

	if (ElimBotSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ElimBotSound,
			ElimBotLocation
		);
	}

	bool bHideSniperScope = IsLocallyControlled() &&
		Combat &&
		Combat->bAiming &&
		Combat->EquippedWeapon &&
		Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}

	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}

	GetWorldTimerManager().SetTimer(ElimTimer, this, &ABlasterCharacter::ElimTimerFinished, ElimDelay);
}

void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void ABlasterCharacter::DropOrDestroyWeapons()
{
	if (Combat == nullptr) return;

	if (Combat->EquippedWeapon)
	{
		DropOrDestroyWeapon(Combat->EquippedWeapon);
	}

	if (Combat->SecondaryWeapon)
	{
		DropOrDestroyWeapon(Combat->SecondaryWeapon);
	}

	if (Combat->TheFlag)
	{
		Combat->TheFlag->Dropped();
	}
}

void ABlasterCharacter::ElimTimerFinished()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if (BlasterGameMode && !bLeftGame)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void ABlasterCharacter::ServerLeaveGame_Implementation()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if (BlasterGameMode && BlasterPlayerState)
	{
		BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
	}
}

void ABlasterCharacter::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) return;
	if (CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetMesh(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 130.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void ABlasterCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

void ABlasterCharacter::SetTeamColor(ETeam Team)
{
	if (GetMesh() == nullptr || OriginalMaterial == nullptr) return;

	switch (Team)
	{
	case ETeam::ET_NotTeam:
		GetMesh()->SetMaterial(0, OriginalMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;
	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0, RedMaterial);
		DissolveMaterialInstance = RedDissolveMatInst;
		break;
	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0, BlueMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;
	}
}

void ABlasterCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::EquipButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		if (Combat->CombatState != ECombatState::ECS_Unoccupied) return;

		ServerEquipButtonPressed();
		if (Combat->ShouldSwapWeapons() && !HasAuthority() && OverlappingWeapon == nullptr)
		{
			Combat->CombatState = ECombatState::ECS_SwappingWeapons;
			PlaySwapMontage();
			bFinishedSwapping = false;
		}
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else if (Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag) return;
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->ReloadButtonPressed();
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag) return;
	if (bDisableGameplay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::AimButtonOnPressed()
{
	if (Combat && Combat->bHoldingTheFlag) return;
	if (bDisableGameplay || bElimmed) return;
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonOnReleased()
{
	if (Combat && Combat->bHoldingTheFlag) return;
	if (bDisableGameplay || bElimmed) return;
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->bHoldingTheFlag)
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (GetLocalRole() <= ENetRole::ROLE_SimulatedProxy) return;
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	FVector Velocity = GetVelocity();
	float Speed = Velocity.Size2D();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // Standing still, not jumping
	{
		FRotator AimRotation = UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(), GetActorRotation());
		FRotator DeltaRotation = UKismetMathLibrary::RInterpTo(FRotator(AO_Pitch, AO_Yaw, 0.f), AimRotation, DeltaTime, InterpSpeed);
		AO_Yaw = FMath::ClampAngle(DeltaRotation.Yaw, -90.0f, 90.0f);
		AO_Pitch = FMath::ClampAngle(DeltaRotation.Pitch, -90.0f, 90.0f);
		bUseControllerRotationYaw = false;
		TurnInPlace(DeltaTime);
	}

	if (Speed > 0.f || bIsInAir)
	{
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
}

void ABlasterCharacter::Jump()
{
	if (Combat && Combat->bHoldingTheFlag) return;
	if (bDisableGameplay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}


void ABlasterCharacter::FireButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag) return;
	if (bDisableGameplay) return;
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (Combat && Combat->bHoldingTheFlag) return;
	if (bDisableGameplay) return;
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::GrenadeButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag) return;
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->ThrowGrenade();
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if (bElimmed || BlasterGameMode == nullptr) return; //不然一直接收伤害

	Damage = BlasterGameMode->CalculateDamage(InstigatedBy, Controller, Damage);
	float DamageToHealth = Damage;
	if (Shield > 0.0f)
	{
		if (Shield > Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.0f, MaxShield);
			DamageToHealth = 0.0f;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.0f, Damage);
			Shield = 0.0f;
		}
	}

	Health = FMath::Clamp(Health - DamageToHealth, 0.0f, MaxHealth);

	PlayHitReactMontage();
	UpdateHUDHealth();
	UpdateHUDShield();

	//if (Health == 0.0f)
	if (FMath::IsNearlyZero(Health, 0.001f))
	{
		if (BlasterGameMode)
		{
			BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterController;
			ABlasterPlayerController* AttackController = Cast<ABlasterPlayerController>(InstigatedBy);
			BlasterGameMode->PlayerEliminated(this, BlasterController, AttackController);
		}
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw >= 90.0f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw <= -90.0f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		float Interp = AO_Yaw;
		AO_Yaw = FMath::FInterpTo(AO_Yaw, 0.0f, DeltaTime, TurningInterpSpeed);
		AddActorLocalRotation(FRotator(0.f, Interp - AO_Yaw, 0.f));
		if (FMath::Abs(AO_Yaw) < 15.0f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	// 这里应该没用了吧，不是很确定,因为在本地设置OverlappingWeapon了，不需要复制了吧，除非只在服务器端检查重叠
	/*if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}*/
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;

	if ((GetActorLocation() - FollowCamera->GetComponentLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (GetEquippedWeapon() && GetEquippedWeapon()->GetWeaponMesh())
		{
			GetEquippedWeapon()->GetWeaponMesh()->bOwnerNoSee = true;
		}
		if (GetSecondaryWeapon() && GetSecondaryWeapon()->GetWeaponMesh())
		{
			GetSecondaryWeapon()->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (GetEquippedWeapon() && GetEquippedWeapon()->GetWeaponMesh())
		{
			GetEquippedWeapon()->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if (GetSecondaryWeapon() && GetSecondaryWeapon()->GetWeaponMesh())
		{
			GetSecondaryWeapon()->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterController;
	if (BlasterController)
	{
		BlasterController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::UpdateHUDShield()
{
	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterController;
	if (BlasterController)
	{
		BlasterController->SetHUDShield(Shield, MaxShield);
	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterController;
	if (BlasterController && Combat && Combat->EquippedWeapon)
	{
		BlasterController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetCurrentAmmo(), Combat->EquippedWeapon->GetMaxAmmo());
	}
}

void ABlasterCharacter::SpawnDefaultWeapon()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	UWorld* World = GetWorld();
	if (BlasterGameMode && World && !bElimmed && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		check(StartingWeapon);

		StartingWeapon->bDestroyWeapon = true;
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

AWeapon* ABlasterCharacter::GetSecondaryWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->SecondaryWeapon;
}

const FVector ABlasterCharacter::GetHitTarget() const
{
	if(Combat == nullptr) return FVector();
	return Combat->HitPoint;
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

bool ABlasterCharacter::IsLocallyReloading() const
{
	if (Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}

bool ABlasterCharacter::IsHoldingTheFlag() const
{
	if (Combat == nullptr) return false;
	return Combat->bHoldingTheFlag;
}

ETeam ABlasterCharacter::GetTeam()
{
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if (BlasterPlayerState == nullptr) return ETeam::ET_NotTeam;
	return BlasterPlayerState->GetTeam();
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ThisClass::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ThisClass::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ThisClass::MoveRight);

	PlayerInputComponent->BindAxis("TurnRight", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ThisClass::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ThisClass::CrouchButtonPressed);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ThisClass::AimButtonOnPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ThisClass::AimButtonOnReleased);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ThisClass::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ThisClass::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ABlasterCharacter::GrenadeButtonPressed);
}
