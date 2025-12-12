#pragma once
#include "../../pch.h"

class FortPlayerControllerAthena
{
public:
	static FortPlayerControllerAthena* Get()
	{
		static FortPlayerControllerAthena* Instance = new FortPlayerControllerAthena();
		return Instance;
	}
public:
	static void Hook();
	static bool HasStreamingLevelsCompletedLoadingUnLoading(AFortPlayerControllerAthena* PC, __int64 a2);
};