#pragma once
#include "../../pch.h"

class UnrealEngine
{
public:
	static UnrealEngine* Get()
	{
		static UnrealEngine* Instance = new UnrealEngine();
		return Instance;
	}
public:
	static void Hook();
	static float GetMaxTickRate();
};