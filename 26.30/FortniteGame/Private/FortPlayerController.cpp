#include "../Public/FortPlayerController.h"
#include "../Public/FortInventoryServiceComponent.h"
#include "../Public/FortAbilitySystemComponentAthena.h"
#include "../Public/FortLootPackage.h"

inline static FortLootPackage* LootPackage = FortLootPackage::Get();
inline static FortInventoryServiceComponent* InventoryService = FortInventoryServiceComponent::Get();

inline static Utils* Util = Utils::Get();

void FortPlayerController::Hook()
{
	Util->HookVTable<AFortPlayerControllerAthena>(557, ServerExecuteInventoryItem);
	Util->HookVTable<AFortPlayerControllerAthena>(640, ServerLoadingScreenDropped);
	Util->HookVTable<AFortPlayerControllerAthena>(639, ServerReadyToStartMatch);
	Util->HookVTable<AFortPlayerControllerAthena>(568, ServerAttemptInventoryDrop);
}

void FortPlayerController::ServerExecuteInventoryItem(AFortPlayerController* PC, FGuid ItemGuid)
{
	if (!PC || PC->IsInAircraft())
		return;

	FFortItemEntry* ItemEntry = FortInventoryServiceComponent::Get()->FindEntry(PC->Cast<AFortPlayerControllerAthena>(), ItemGuid);
	if (!ItemEntry)
		return;

	PC->GetPlayerPawn()->EquipWeaponDefinition(ItemEntry->ItemDefinition->Cast<UFortWeaponItemDefinition>(), ItemEntry->ItemGuid, ItemEntry->TrackerGuid, false);
}

void FortPlayerController::ServerLoadingScreenDropped(AFortPlayerController* PC)
{
	if (!PC)
		return;

	AFortPlayerStateAthena* PlayerState = PC->PlayerState->Cast<AFortPlayerStateAthena>();
	if (!PlayerState)
		return;

	FortAbilitySystemComponentAthena::Get()->Init(PlayerState->AbilitySystemComponent);
}

void FortPlayerController::ServerReadyToStartMatch(AFortPlayerController* PC)
{
	static bool bInit = false;
	static UWorld* World = UWorld::GetWorld();
	static AFortGameStateAthena* GameState = World->GameState->Cast<AFortGameStateAthena>();

	if (!bInit)
	{
		bInit = true;
		LootPackage->Init(World, GameState);
	}
}

void FortPlayerController::ServerAttemptInventoryDrop(AFortPlayerController* PC, FGuid& ItemGuid, int32 Count, bool bTrash)
{
	if (!PC)
		return;

	Util->Logger(Info, "ServerAttemptInventoryDrop", std::format("bTrash={}", bTrash).c_str());

	FFortItemEntry* ItemEntry = InventoryService->FindEntry(PC->Cast<AFortPlayerControllerAthena>(), ItemGuid);
	if (!ItemEntry)
		return;

	if (!bTrash)
	{
		LootPackage->SpawnPickup(LootPackage->GetLocationSpawnDrop(PC->Pawn), ItemEntry->ItemDefinition, Count, ItemEntry->LoadedAmmo, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::LootDrop, PC->MyFortPawn);
		InventoryService->RemoveItem(PC->Cast<AFortPlayerControllerAthena>(), ItemGuid, Count);
	}
	else
	{
		InventoryService->RemoveItem(PC->Cast<AFortPlayerControllerAthena>(), ItemGuid, Count);
	}
}
