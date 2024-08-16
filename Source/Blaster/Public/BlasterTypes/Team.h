#pragma once

UENUM(BlueprintType)
enum class ETeam : uint8
{
	ET_NotTeam UMETA(DisplayName = "NotTeam"),
	ET_RedTeam UMETA(DisplayName = "RedTeam"),
	ET_BlueTeam UMETA(DisplayName = "BlueTeam"),

	ET_MAX UMETA(DisplayName = "DefaultMax")
};