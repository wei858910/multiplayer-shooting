// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "JoinRoomSessionCallbackProxy.generated.h"

struct FBlueprintRoomSessionResult;
class UMultiplayerSessionsSubsystem;
/**
 * 
 */
UCLASS() // MinimalAPI宏和MULTIPLAYERSESSIONS_API不能一起用
class MULTIPLAYERSESSIONS_API UJoinRoomSessionCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnSuccess;

	// Called when there is an unsuccessful join
	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;

	// Joins a remote session with the default online subsystem
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "Online|Session")
	static UJoinRoomSessionCallbackProxy* JoinRoomSession(UObject* WorldContextObject, const FBlueprintRoomSessionResult& SearchResult);

	// UOnlineBlueprintCallProxyBase interface
	virtual void Activate() override;
	
private:
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

private:
	FOnlineSessionSearchResult OnlineSearchResult;

	FDelegateHandle JoinSessionDelegateHandle;

	UObject* WorldContextObject;

	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
};
