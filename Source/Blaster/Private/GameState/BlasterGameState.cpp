// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/BlasterPlayerState.h"
#include "PlayerController/BlasterPlayerController.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayer);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
}

void ABlasterGameState::UpdateToScore(ABlasterPlayerState* ScoringPlayer)
{
	if (TopScoringPlayer.Num() == 0)
	{
		TopScore = ScoringPlayer->GetScore();
		TopScoringPlayer.Add(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayer.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayer.Empty();
		TopScoringPlayer.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void ABlasterGameState::RedTeamScores()
{
	++RedTeamScore;

	ABlasterPlayerController* BlasterPController = GetWorld()->GetFirstPlayerController<ABlasterPlayerController>();
	if (BlasterPController)
	{
		BlasterPController->SetHUDRedScore(RedTeamScore);
	}
}

void ABlasterGameState::BlueTeamScores()
{
	++BlueTeamScore;

	ABlasterPlayerController* BlasterPController = GetWorld()->GetFirstPlayerController<ABlasterPlayerController>();
	if (BlasterPController)
	{
		BlasterPController->SetHUDBlueScore(BlueTeamScore);
	}
}

void ABlasterGameState::OnRep_RedTeamScore()
{
	ABlasterPlayerController* BlasterPController = GetWorld()->GetFirstPlayerController<ABlasterPlayerController>();
	if (BlasterPController)
	{
		BlasterPController->SetHUDRedScore(RedTeamScore);
	}
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
	ABlasterPlayerController* BlasterPController = GetWorld()->GetFirstPlayerController<ABlasterPlayerController>();
	if (BlasterPController)
	{
		BlasterPController->SetHUDBlueScore(BlueTeamScore);
	}
}
