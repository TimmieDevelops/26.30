#pragma once
#include "../../pch.h"

class FortLootPackage
{
public:
	static FortLootPackage* Get()
	{
		static FortLootPackage* Instance = new FortLootPackage();
		return Instance;
	}
public:
	void Init(UWorld* World, AFortGameStateAthena* GameState);
	std::vector<FFortItemEntry*> PickLootDrops(FName TierGroupName, int WorldLevel);
	AFortPickup* SpawnPickup(FVector Location, UFortItemDefinition* ItemDefinition, int Count, int LoadedAmmo = 0, EFortPickupSourceTypeFlag TypeFlag = EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource SpawnSource = EFortPickupSpawnSource::Unset, AFortPawn* Pawn = nullptr);
	FVector GetLocationSpawnDrop(AActor* Actor);
private:
	FFortLootTierData* SelectLootTier(const std::vector<FFortLootTierData*>& OutLootTiers);
	FFortLootPackageData* SelectLootPackage(const std::vector<FFortLootPackageData*>& OutLootPackages);
};