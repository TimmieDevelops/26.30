#pragma once
#include "../../pch.h"

class World
{
public:
	static World* Get()
	{
		static World* Instance = new World();
		return Instance;
	}
public:
	static void Hook();
	static ENetMode InternalGetNetMode(UWorld* World);
};