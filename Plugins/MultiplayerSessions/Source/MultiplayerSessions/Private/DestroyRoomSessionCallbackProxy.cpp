// Fill out your copyright notice in the Description page of Project Settings.


#include "DestroyRoomSessionCallbackProxy.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerSessionsSubsystem.h"

UDestroyRoomSessionCallbackProxy* UDestroyRoomSessionCallbackProxy::DestroyRoomSession(UObject* WorldContextObject)
{
	UDestroyRoomSessionCallbackProxy* Proxy = NewObject<UDestroyRoomSessionCallbackProxy>();
	Proxy->WorldContextObject = WorldContextObject;
	return Proxy;
}

void UDestroyRoomSessionCallbackProxy::Activate()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem)
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UDestroyRoomSessionCallbackProxy::OnDestroySession);
			MultiplayerSessionsSubsystem->DestroySession();
			return;
		}
	}
	OnFailure.Broadcast();
}

void UDestroyRoomSessionCallbackProxy::OnDestroySession(bool bWasSuccessful)
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
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UDestroyRoomSessionCallbackProxy::OnDestroySession);
	}
}
