// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateToScore(class ABlasterPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayer;

	/**
	* Teams
	*/

	UPROPERTY()
	TArray<ABlasterPlayerState*> RedTeam;

	UPROPERTY()
	TArray<ABlasterPlayerState*> BlueTeam;

	void RedTeamScores();
	void BlueTeamScores();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UFUNCTION()
	void OnRep_BlueTeamScore();

private:
	float TopScore = 0.f;

public:
	FORCEINLINE float GetRedTeamScore() const { return RedTeamScore; }
	FORCEINLINE float GetBlueTeamScore() const { return BlueTeamScore; }
};
