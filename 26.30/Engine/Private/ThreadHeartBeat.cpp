#include "../Public/ThreadHeartBeat.h"

void ThreadHeartBeat::Hook()
{
	MH_CreateHook((LPVOID)(GetImageBase() + 0x3E53000), CheckCheckpointHeartBeat, nullptr);
}

uint32 ThreadHeartBeat::CheckCheckpointHeartBeat(double& OutHangDuration)
{
	return -1;
}
