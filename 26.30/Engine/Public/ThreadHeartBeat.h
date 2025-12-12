#pragma once
#include "../../pch.h"

class ThreadHeartBeat
{
public:
	static ThreadHeartBeat* Get()
	{
		static ThreadHeartBeat* Instance = new ThreadHeartBeat();
		return Instance;
	}
public:
	static void Hook();
	static uint32 CheckCheckpointHeartBeat(double& OutHangDuration);
};