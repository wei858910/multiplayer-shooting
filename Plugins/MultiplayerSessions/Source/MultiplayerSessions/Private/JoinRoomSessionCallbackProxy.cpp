// Fill out your copyright notice in the Description page of Project Settings.


#include "JoinRoomSessionCallbackProxy.h"
#include "FindRoomSessionsCallbackProxy.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerSessionsSubsystem.h"

UJoinRoomSessionCallbackProxy* UJoinRoomSessionCallbackProxy::JoinRoomSession(UObject* WorldContextObject, const FBlueprintRoomSessionResult& SearchResult)
{
	UJoinRoomSessionCallbackProxy* Proxy = NewObject<UJoinRoomSessionCallbackProxy>();
	Proxy->WorldContextObject = WorldContextObject;
	Proxy->OnlineSearchResult = SearchResult.OnlineResult;
	return Proxy;
}

void UJoinRoomSessionCallbackProxy::Activate()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = MultiplayerSessionsSubsystem == nullptr ? GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>() : MultiplayerSessionsSubsystem;
		if (MultiplayerSessionsSubsystem)
		{
			JoinSessionDelegateHandle = MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &UJoinRoomSessionCallbackProxy::OnJoinSession);
			MultiplayerSessionsSubsystem->JoinSession(OnlineSearchResult);
			return;
		}
	}
	OnFailure.Broadcast();
}

void UJoinRoomSessionCallbackProxy::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
		if (GameInstance)
		{
			MultiplayerSessionsSubsystem = MultiplayerSessionsSubsystem == nullptr ? GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>() : MultiplayerSessionsSubsystem;
			if (MultiplayerSessionsSubsystem)
			{
				FString ConnectString;
				if (MultiplayerSessionsSubsystem->GetResolvedConnectString(NAME_GameSession, ConnectString))
				{
					APlayerController* PlayerController = GameInstance->GetFirstLocalPlayerController();
					if (PlayerController)
					{
						PlayerController->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
						OnSuccess.Broadcast();
						return;
					}
				}
			}
		}
	}

	OnFailure.Broadcast();

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.Remove(JoinSessionDelegateHandle);
	}
}
