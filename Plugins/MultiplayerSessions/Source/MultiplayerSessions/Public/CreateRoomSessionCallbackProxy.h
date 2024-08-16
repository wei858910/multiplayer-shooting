// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#include "CreateRoomSessionCallbackProxy.generated.h"

class UMultiplayerSessionsSubsystem;
/**
 * 
 */
UCLASS() // MinimalAPI宏和MULTIPLAYERSESSIONS_API不能一起用
class MULTIPLAYERSESSIONS_API UCreateRoomSessionCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "Online|Session")
	static UCreateRoomSessionCallbackProxy* CreateRoomSession(UObject* WorldContextObject, 
		FString MatchType,
		int32 PublicConnections, 
		bool bUseLAN);

	virtual void Activate() override;

private:
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

private:
	FString MatchType;

	// Number of public connections
	int NumPublicConnections;

	// Whether or not to search LAN
	bool bUseLAN;

	UObject* WorldContextObject;

	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
};
