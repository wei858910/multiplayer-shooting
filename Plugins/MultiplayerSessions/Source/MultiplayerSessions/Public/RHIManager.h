// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RHIManager.generated.h"

UENUM(BlueprintType)
enum class ERHI : uint8
{
//#if PLATFORM_WINDOWS  
	DX11	UMETA(DisplayName = "Dx11"),
	DX12	UMETA(DisplayName = "Dx12"),
//#endif

//#if PLATFORM_MAC
	METAL	UMETA(DisplayName = "Metal"),
//#endif

//#if PLATFORM_LINUX || PLATFORM_WINDOWS || PLATFORM_ANDROID
	VULKAN	UMETA(DisplayName = "Vulkan"),
//#endif	

//#if PLATFORM_LINUX || PLATFORM_ANDROID
	OPENGL	UMETA(DisplayName = "OpenGL"),
//#endif
};

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API URHIManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "RHI")
	static void SetRHI(ERHI RHI);

	UFUNCTION(BlueprintPure, Category = "RHI")
	static ERHI GetCurrentRHI();
};
