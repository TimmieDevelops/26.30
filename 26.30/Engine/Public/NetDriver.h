#pragma once
#include "../../pch.h"

class NetDriver
{
public:
	static NetDriver* Get()
	{
		static NetDriver* Instance = new NetDriver();
		return Instance;
	}
private:
	inline static void* (*TickFlushOG)(UNetDriver* Driver, float DeltaSeconds);
public:
	static void Hook();
	static void* TickFlush(UNetDriver* Driver, float DeltaSeconds);
	static void SendClientMoveAdjustments(UNetDriver* Driver);
};