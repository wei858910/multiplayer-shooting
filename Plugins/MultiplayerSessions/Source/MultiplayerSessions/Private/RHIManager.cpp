// Fill out your copyright notice in the Description page of Project Settings.


#include "RHIManager.h"
#include "Kismet/GameplayStatics.h"

void URHIManager::SetRHI(ERHI RHI)
{
	FString CurrentPlatform = UGameplayStatics::GetPlatformName();
	FString DefaultGraphicsRHI;

	if (CurrentPlatform == "Windows")
	{
		GConfig->GetString(TEXT("/Script/WindowsTargetPlatform.WindowsTargetSettings"), TEXT("DefaultGraphicsRHI"), DefaultGraphicsRHI, GEngineIni);

		switch (RHI)
		{
		case ERHI::DX11:
			if (DefaultGraphicsRHI != TEXT("DefaultGraphicsRHI_DX11"))
			{
				GConfig->SetString(TEXT("/Script/WindowsTargetPlatform.WindowsTargetSettings"), TEXT("DefaultGraphicsRHI"), TEXT("DefaultGraphicsRHI_DX11"), GEngineIni);
				GConfig->Flush(true, GEngineIni);
			}
			break;
		case ERHI::DX12:
			if (DefaultGraphicsRHI != TEXT("DefaultGraphicsRHI_DX12"))
			{
				GConfig->SetString(TEXT("/Script/WindowsTargetPlatform.WindowsTargetSettings"), TEXT("DefaultGraphicsRHI"), TEXT("DefaultGraphicsRHI_DX12"), GEngineIni);
				GConfig->Flush(true, GEngineIni);
			}
			break;
		case ERHI::VULKAN:
			if (DefaultGraphicsRHI != TEXT("DefaultGraphicsRHI_Vulkan"))
			{
				GConfig->SetString(TEXT("/Script/WindowsTargetPlatform.WindowsTargetSettings"), TEXT("DefaultGraphicsRHI"), TEXT("DefaultGraphicsRHI_Vulkan"), GEngineIni);
				GConfig->Flush(true, GEngineIni);
			}
			break;
		default:
			break;
		}
	}
}

ERHI URHIManager::GetCurrentRHI()
{
	FString CurrentPlatform = UGameplayStatics::GetPlatformName();
	FString DefaultGraphicsRHI;

	if (DefaultGraphicsRHI == TEXT("DefaultGraphicsRHI_DX11"))
	{
		return ERHI::DX11;
	}
	else if(DefaultGraphicsRHI == TEXT("DefaultGraphicsRHI_DX12"))
	{
		return ERHI::DX12;
	}
	else if (DefaultGraphicsRHI == TEXT("DefaultGraphicsRHI_Vulkan"))
	{
		return ERHI::VULKAN;
	}
	else if (DefaultGraphicsRHI == TEXT("DefaultGraphicsRHI_OpenGL"))
	{
		return ERHI::OPENGL;
	}
	else if (DefaultGraphicsRHI == TEXT("DefaultGraphicsRHI_Metal"))
	{
		return ERHI::METAL;
	}
	return ERHI::VULKAN;
}
