// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#include "DestroyRoomSessionCallbackProxy.generated.h"

class UMultiplayerSessionsSubsystem;
/**
 * 
 */
UCLASS() // MinimalAPI宏和MULTIPLAYERSESSIONS_API不能一起用
class MULTIPLAYERSESSIONS_API UDestroyRoomSessionCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "Online|Session")
	static UDestroyRoomSessionCallbackProxy* DestroyRoomSession(UObject* WorldContextObject);

	virtual void Activate() override;

private:
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

private:
	UObject* WorldContextObject;

	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
};
