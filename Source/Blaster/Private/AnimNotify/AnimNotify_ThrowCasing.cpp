// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify/AnimNotify_ThrowCasing.h"
#include "Character/BlasterCharacter.h"
#include "Weapon/Weapon.h"

//void UAnimNotify_ThrowCasing::Notify(USkeletalMeshComponent* SkeletalMeshComp, UAnimSequenceBase* Animation)
//{
//	Super::Notify(SkeletalMeshComp, Animation);
//}

void UAnimNotify_ThrowCasing::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
}
