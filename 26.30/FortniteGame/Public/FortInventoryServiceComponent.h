#pragma once
#include "../../pch.h"

class FortInventoryServiceComponent
{
public:
	static FortInventoryServiceComponent* Get()
	{
		static FortInventoryServiceComponent* Instance = new FortInventoryServiceComponent();
		return Instance;
	}
public:
	static void Hook();
	static void SetupInventoryServiceComponent(UFortControllerComponent_InventoryService* InventoryService);
public:
	static UFortWorldItem* GiveItem(AFortPlayerControllerAthena* PC, UFortItemDefinition* ItemDefinition, int Count, int LoadedAmmo = 0);
	static void Update(AFortPlayerControllerAthena* PC, FFortItemEntry* ItemEntry = nullptr);
	static FFortItemEntry* FindEntry(AFortPlayerControllerAthena* PC, FGuid& ItemGuid);
	static FFortItemEntry* FindEntry(AFortPlayerControllerAthena* PC, UFortItemDefinition* ItemDefinition);
	static void AddItem(AFortPlayerControllerAthena* PC, UFortItemDefinition* ItemDefinition, int Count, int LoadedAmmo = 0);
	static void RemoveItem(AFortPlayerControllerAthena* PC, FGuid& ItemGuid, int Count);
};