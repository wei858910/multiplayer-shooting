// Fill out your copyright notice in the Description page of Project Settings.


#include "CreateRoomSessionCallbackProxy.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerSessionsSubsystem.h"

UCreateRoomSessionCallbackProxy* UCreateRoomSessionCallbackProxy::CreateRoomSession(UObject* WorldContextObject, FString MatchType, int32 PublicConnections, bool bUseLAN)
{
	UCreateRoomSessionCallbackProxy* Proxy = NewObject<UCreateRoomSessionCallbackProxy>();
	Proxy->WorldContextObject = WorldContextObject;
	Proxy->NumPublicConnections = PublicConnections;
	Proxy->bUseLAN = bUseLAN;
	Proxy->MatchType = MatchType;
	return Proxy;
}

void UCreateRoomSessionCallbackProxy::Activate()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem)
		{
			MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &UCreateRoomSessionCallbackProxy::OnCreateSession);
			MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType, bUseLAN);
			return;
		}
	}
	OnFailure.Broadcast();
}

void UCreateRoomSessionCallbackProxy::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.RemoveDynamic(this, &UCreateRoomSessionCallbackProxy::OnCreateSession);
	}
}
