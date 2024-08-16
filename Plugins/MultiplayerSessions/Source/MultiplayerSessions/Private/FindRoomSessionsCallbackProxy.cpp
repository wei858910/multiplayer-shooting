// Fill out your copyright notice in the Description page of Project Settings.


#include "FindRoomSessionsCallbackProxy.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerSessionsSubsystem.h"

UFindRoomSessionsCallbackProxy* UFindRoomSessionsCallbackProxy::FindRoomSessions(UObject* WorldContextObject, FString TypeOfMatch, int32 MaxResults, bool bUseLAN)
{
	UFindRoomSessionsCallbackProxy* Proxy = NewObject<UFindRoomSessionsCallbackProxy>();
	Proxy->WorldContextObject = WorldContextObject;
	Proxy->MaxResults = MaxResults;
	Proxy->bUseLAN = bUseLAN;
	Proxy->MatchType = TypeOfMatch;
	return Proxy;
}

void UFindRoomSessionsCallbackProxy::Activate()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem)
		{
			FindSessionsDelegateHandle = MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &UFindRoomSessionsCallbackProxy::OnFindSessions);
			MultiplayerSessionsSubsystem->FindSessions(MaxResults, bUseLAN);
			return;
		}
	}
	OnFailure.Broadcast(TArray<FBlueprintRoomSessionResult>());
}

void UFindRoomSessionsCallbackProxy::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	TArray<FBlueprintRoomSessionResult> Results;
	for (const auto& SessionResult : SessionResults)
	{
		//创建MatchTypeValue，这个将用来接受获取的value值
		FString MatchTypeValue;
		//这里找到key值后，将其value值放入MatchTypeValue里面
		SessionResult.Session.SessionSettings.Get(FName("MatchType"), MatchTypeValue);
		if (MatchTypeValue == MatchType)
		{
			Results.Add(FBlueprintRoomSessionResult{ SessionResult });
		}
	}

	if (bWasSuccessful)
	{
		OnSuccess.Broadcast(Results);
	}
	else
	{
		OnFailure.Broadcast(TArray<FBlueprintRoomSessionResult>());
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.Remove(FindSessionsDelegateHandle);
	}
}

int32 UFindRoomSessionsCallbackProxy::GetPingInMs(const FBlueprintRoomSessionResult& Result)
{
	return Result.OnlineResult.PingInMs;
}

FString UFindRoomSessionsCallbackProxy::GetServerName(const FBlueprintRoomSessionResult& Result)
{
	return Result.OnlineResult.Session.OwningUserName;
}

int32 UFindRoomSessionsCallbackProxy::GetCurrentPlayers(const FBlueprintRoomSessionResult& Result)
{
	return Result.OnlineResult.Session.SessionSettings.NumPublicConnections - Result.OnlineResult.Session.NumOpenPublicConnections;
}

int32 UFindRoomSessionsCallbackProxy::GetMaxPlayers(const FBlueprintRoomSessionResult& Result)
{
	return Result.OnlineResult.Session.SessionSettings.NumPublicConnections;
}