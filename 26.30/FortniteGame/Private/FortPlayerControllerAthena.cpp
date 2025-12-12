#include "../Public/FortPlayerControllerAthena.h"

void FortPlayerControllerAthena::Hook()
{
	MH_CreateHook((LPVOID)(GetImageBase() + 0x79A3F38), HasStreamingLevelsCompletedLoadingUnLoading, nullptr);
}

bool FortPlayerControllerAthena::HasStreamingLevelsCompletedLoadingUnLoading(AFortPlayerControllerAthena* PC, __int64 a2)
{
	return true;
}
