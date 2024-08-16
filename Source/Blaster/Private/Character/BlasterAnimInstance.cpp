// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterAnimInstance.h"
#include "Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Weapon/Weapon.h"
#include "BlasterTypes/CombatState.h"
#include "BlasterComponents/CombatComponent.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}

	if (BlasterCharacter == nullptr) return;

	Speed = BlasterCharacter->GetVelocity().Size2D();

	bElimmed = BlasterCharacter->IsElimmed();

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f ? true : false;
	
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();

	bUseFABRIK = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	bool bFABRIKOverride = BlasterCharacter->IsLocallyControlled() &&
		BlasterCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade && 
		BlasterCharacter->bFinishedSwapping;
	if (bFABRIKOverride)
	{
		bUseFABRIK = !BlasterCharacter->IsLocallyReloading();
	}

	bUseAimOffsets = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGameplay();

	bTransformRightHand = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGameplay();

	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();

	/*bIsCrouched = BlasterCharacter->bIsCrouched;*/
	bIsCrouched = BlasterCharacter->GetCharacterMovement()->IsCrouching();

	TurningInPlace = BlasterCharacter->GetTurningInPlace();

	bHoldingTheFlag = BlasterCharacter->IsHoldingTheFlag();

	/*	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
		YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
	*/
	bIsAiming = BlasterCharacter->IsAiming();
	YawOffset = UKismetAnimationLibrary::CalculateDirection(BlasterCharacter->GetVelocity(), BlasterCharacter->GetActorRotation());

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.0f);
	Lean = FMath::Clamp(Interp, -90.0f, 90.f);
	
	/*
	FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(BlasterCharacter->GetControlRotation(), BlasterCharacter->GetActorRotation());
	FRotator DeltaRot = UKismetMathLibrary::RInterpTo(FRotator(AO_Pitch, AO_Yaw, 0), DeltaRotation, DeltaTime, 15.0f);
	AO_Pitch = FMath::ClampAngle(DeltaRot.Pitch, -90, 90);
	AO_Yaw = FMath::ClampAngle(DeltaRot.Yaw, -90, 90);
	*/
	
	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && BlasterCharacter->GetMesh() && EquippedWeapon->GetWeaponMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (BlasterCharacter->IsLocallyControlled())
		{
			bIsLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));

			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.0f);
		}
	}
}

void UBlasterAnimInstance::AnimNotify_ReloadFinished(UAnimNotify* Notify)
{
	if (BlasterCharacter == nullptr || BlasterCharacter->GetCombat() == nullptr) return;

	BlasterCharacter->GetCombat()->FinishedReloading();
}

void UBlasterAnimInstance::AnimNotify_Shell(UAnimNotify* Notify)
{
	if (BlasterCharacter == nullptr || BlasterCharacter->GetCombat() == nullptr) return;
	BlasterCharacter->GetCombat()->ShotgunShellReload();
}
