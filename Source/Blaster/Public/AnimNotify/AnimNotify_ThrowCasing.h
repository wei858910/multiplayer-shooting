// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_ThrowCasing.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UAnimNotify_ThrowCasing : public UAnimNotify
{
	GENERATED_BODY()
public:
	// ¹ýÊ±ÁË
	//virtual void Notify(USkeletalMeshComponent* SkeletalMeshComp, UAnimSequenceBase* Animation) override; 
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
