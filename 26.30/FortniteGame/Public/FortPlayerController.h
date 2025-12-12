#pragma once
#include "../../pch.h"

class FortPlayerController
{
public:
	static FortPlayerController* Get()
	{
		static FortPlayerController* Instance = new FortPlayerController();
		return Instance;
	}
public:
	static void Hook();
	static void ServerExecuteInventoryItem(AFortPlayerController* PC, FGuid ItemGuid);
	static void ServerLoadingScreenDropped(AFortPlayerController* PC);
	static void ServerReadyToStartMatch(AFortPlayerController* PC);
	static void ServerAttemptInventoryDrop(AFortPlayerController* PC, FGuid& ItemGuid, int32 Count, bool bTrash);
};