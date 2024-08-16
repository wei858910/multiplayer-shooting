// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"

ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	InitialLifeSpan = 5.0f;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true); // 设置是否开启物理模拟
	CasingMesh->SetEnableGravity(true);	// 开启 Generated Hit Event
	CasingMesh->SetNotifyRigidBodyCollision(true);

	ShellEjectionImpulse = 15.0f;
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();

	FVector Forward = GetActorForwardVector();
	FVector FR = Forward + GetActorRightVector();
	FR = UKismetMathLibrary::Normal(FR);
	float t = FMath::RandRange(0, 1);
	FR = (1 - t)* Forward + t * FR;
	FR = UKismetMathLibrary::Normal(FR);
	CasingMesh->AddImpulse(FR * FMath::RandRange(ShellEjectionImpulse / 2.0f, ShellEjectionImpulse));
}

void ACasing::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (CasingMesh)
	{
		CasingMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	}
}

void ACasing::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	CasingMesh->OnComponentHit.RemoveDynamic(this, &ThisClass::OnHit);

	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
}
