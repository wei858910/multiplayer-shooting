#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/BlasterHUD.h"
#include "BlasterTypes/CombatState.h"
#include "Weapon/WeaponTypes.h"
#include "CombatComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class ABlasterCharacter;

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void EquipWeapon(class AWeapon* WeaponToEquip);
	void SwapWeapons();

	void SetAiming(bool bIsAiming);

	void FinishedReloading();
	void FireButtonPressed(bool bPressed);
	void ShotgunShellReload();
	void JumpToShotgunEnd();

	void ThrowGrenade();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	void UpdateCarriedAmmo();

	UFUNCTION(BlueprintCallable)
	void FinishSwap();

	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapons();

protected:
	virtual void BeginPlay() override;

	void ReloadButtonPressed();

	UFUNCTION(Server, Reliable)
	void ServerReload();
	void ServerReload_Implementation();

	bool CanFire();

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
	void AttachActorToBackpack(AActor* ActorToAttach);

protected:
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	void ServerSetAiming_Implementation(bool bIsAiming);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetAiming(bool bIsAiming);
	void MulticastSetAiming_Implementation(bool bIsAiming);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& HitTarget, float FireDelay);
	void ServerFire_Implementation(const FVector_NetQuantize& HitTarget, float FireDelay);
	bool ServerFire_Validate(const FVector_NetQuantize& HitTarget, float FireDelay);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& HitTargets, float FireDelay);
	void ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& HitTargets, float FireDelay);
	bool ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& HitTargets, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& HitTarget);
	void MulticastFire_Implementation(const FVector_NetQuantize& HitTarget);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& HitTargets);
	void MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& HitTargets);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	void StartFireTimer();
	void AutoFireReset();
	void HandleReload();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();
	void LocalFire(const FVector_NetQuantize& HitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& HitTargets);

	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachFlagToLeftHand(AWeapon* Flag);
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);
	void ReloadEmptyWeapon();

	void ShowAttachedGrenade(bool bShowGrenade);

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

private:
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;
	UPROPERTY()
	class ABlasterHUD* HUD;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectileGrenade> GrenadeClass;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_TheFlag)
	AWeapon* TheFlag;

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon(AWeapon* OldWeapon);

	UFUNCTION()
	void OnRep_TheFlag();

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	bool bAimButtonPressed = false;

	UFUNCTION()
	void OnRep_Aiming();

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitPoint;

	FHUDPackage HUDPackage;

	/**
	* Aiming and FOV
	*/

	float DefaultFOV;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV = 45.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomInterSpeed = 20.0f;

	void InterpFOV(float DeltaTime);

	FTimerHandle AutoFireTimer;

	bool bCanFire = true;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;

	void UpdateHUDGrenades();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	void InitializeCarriedAmmo();

	// Carried ammo for the currently-equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0;

	bool bLocallyReloading = false;

	UPROPERTY(ReplicatedUsing = OnRep_HoldingTheFlag)
	bool bHoldingTheFlag = false;

	UFUNCTION()
	void OnRep_HoldingTheFlag();

public:	
	FORCEINLINE bool ShouldSwapWeapons() { return EquippedWeapon != nullptr && SecondaryWeapon != nullptr; }
	FORCEINLINE int32 GetGrenades()const { return Grenades; }
};
