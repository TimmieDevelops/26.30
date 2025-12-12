#include "../Public/BuildingContainer.h"
#include "../Public/FortLootPackage.h"

#include "../../Engine/Public/Actor.h"

inline static FortLootPackage* LootPackage = FortLootPackage::Get();

inline static Actor* Actors = Actor::Get();

inline static Utils* Util = Utils::Get();
inline static Globals* Global = Globals::Get();

void BuildingContainer::Hook()
{
	MH_CreateHook((LPVOID)(GetImageBase() + 0x7C4C0A4), SpawnLoot, nullptr);
}

char __fastcall BuildingContainer::SpawnLoot(ABuildingContainer* Container)
{
	if (!Container || Container->bAlreadySearched)
		return 0;

	Util->Logger(Info, "SpawnLoot", std::format("Name={} Search={}", Container->GetName(), Container->SearchLootTierGroup.ToString()));

	Container->bAlreadySearched = true;
	Container->OnRep_bAlreadySearched();
	Container->SearchBounceData.SearchAnimationCount++;
	Container->BounceContainer();

	std::vector<FFortItemEntry*> LootDrops;

	EFortPickupSourceTypeFlag TypeFlag;
	EFortPickupSpawnSource SpawnSource;

	if (Container->GetName().contains("Chest"))
	{
		LootDrops = LootPackage->PickLootDrops(UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaTreasure"), UWorld::GetWorld()->GameState->Cast<AFortGameStateAthena>()->WorldLevel);
		TypeFlag = EFortPickupSourceTypeFlag::Container;
		SpawnSource = EFortPickupSpawnSource::Chest;
	}
	
	if (Container->GetName().contains("Ammo"))
	{
		LootDrops = LootPackage->PickLootDrops(UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaAmmoLarge"), UWorld::GetWorld()->GameState->Cast<AFortGameStateAthena>()->WorldLevel);
		TypeFlag = EFortPickupSourceTypeFlag::Container;
		SpawnSource = EFortPickupSpawnSource::AmmoBox;
	}

	if (Container->GetName().contains("Warmup"))
	{
		LootDrops = LootPackage->PickLootDrops(Container->SearchLootTierGroup, UWorld::GetWorld()->GameState->Cast<AFortGameStateAthena>()->WorldLevel); // calls normal
		TypeFlag = EFortPickupSourceTypeFlag::FloorLoot;
		SpawnSource = EFortPickupSpawnSource::Unset;
	}

	/* crashes
	if (Container->GetName().contains("FoodBox"))
	{
		LootDrops = LootPackage->PickLootDrops(Container->SearchLootTierGroup, UWorld::GetWorld()->GameState->Cast<AFortGameStateAthena>()->WorldLevel);
		TypeFlag = EFortPickupSourceTypeFlag::Container;
		SpawnSource = EFortPickupSpawnSource::Chest;
	}*/

	for (FFortItemEntry* ItemEntry : LootDrops)
	{
		if (!ItemEntry)
			continue;

		FVector Location = Container->K2_GetActorLocation() + (Container->GetActorForwardVector() * Container->LootSpawnLocation_Athena.X) + (Container->GetActorRightVector() * Container->LootSpawnLocation_Athena.Y) + (Container->GetActorUpVector() * Container->LootSpawnLocation_Athena.Z);
		LootPackage->SpawnPickup(Location, ItemEntry->ItemDefinition, ItemEntry->Count, ItemEntry->LoadedAmmo, TypeFlag, SpawnSource);
	}

	return 1;
}
