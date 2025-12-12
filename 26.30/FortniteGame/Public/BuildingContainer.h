#pragma once
#include "../../pch.h"

class BuildingContainer
{
public:
	static BuildingContainer* Get()
	{
		static BuildingContainer* Instance = new BuildingContainer();
		return Instance;
	}
public:
	static void Hook();
	static char __fastcall SpawnLoot(ABuildingContainer* Container);
};