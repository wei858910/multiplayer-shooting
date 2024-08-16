// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#include "FindRoomSessionsCallbackProxy.generated.h"

class UMultiplayerSessionsSubsystem;

USTRUCT(BlueprintType)
struct FBlueprintRoomSessionResult
{
	GENERATED_USTRUCT_BODY()

	FOnlineSessionSearchResult OnlineResult; // 因为它只是一个普通类，没有反射出去，所以要包裹一下
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlueprintFindRoomSessionsResultDelegate, const TArray<FBlueprintRoomSessionResult>&, Results);
/**
 * 
 */
UCLASS() // MinimalAPI宏和MULTIPLAYERSESSIONS_API不能一起用
class MULTIPLAYERSESSIONS_API UFindRoomSessionsCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_BODY()

	// Called when there is a successful query
	UPROPERTY(BlueprintAssignable)
	FBlueprintFindRoomSessionsResultDelegate OnSuccess;

	// Called when there is an unsuccessful query
	UPROPERTY(BlueprintAssignable)
	FBlueprintFindRoomSessionsResultDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "Online|Session")
	static UFindRoomSessionsCallbackProxy* FindRoomSessions(UObject* WorldContextObject, FString TypeOfMatch, int32 MaxResults, bool bUseLAN);

	UFUNCTION(BlueprintPure, Category = "Online|Session")
	static int32 GetPingInMs(const FBlueprintRoomSessionResult& Result);

	UFUNCTION(BlueprintPure, Category = "Online|Session")
	static FString GetServerName(const FBlueprintRoomSessionResult& Result);

	UFUNCTION(BlueprintPure, Category = "Online|Session")
	static int32 GetCurrentPlayers(const FBlueprintRoomSessionResult& Result);

	UFUNCTION(BlueprintPure, Category = "Online|Session")
	static int32 GetMaxPlayers(const FBlueprintRoomSessionResult& Result);

	virtual void Activate() override;

private:
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);

private:
	// Whether or not to search LAN
	bool bUseLAN;

	// Maximum number of results to return
	int MaxResults;

	FString MatchType{ TEXT("FreeForAll") };

	FDelegateHandle FindSessionsDelegateHandle;

	// The world context object in which this call is taking place
	UObject* WorldContextObject;

	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
};
