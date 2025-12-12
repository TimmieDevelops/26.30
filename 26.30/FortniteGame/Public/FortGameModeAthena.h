#pragma once
#include "../../pch.h"

class FortGameModeAthena
{
public:
	static FortGameModeAthena* Get()
	{
		static FortGameModeAthena* Instance = new FortGameModeAthena();
		return Instance;
	}
public:
	static void Hook();
	static bool ReadyToStartMatch(AFortGameModeAthena* GameMode);
	static void HandleMatchHasStarted(AFortGameModeAthena* GameMode);
};