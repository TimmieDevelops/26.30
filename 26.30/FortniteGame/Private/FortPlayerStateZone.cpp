#include "../Public/FortPlayerStateZone.h"
#include "../Public/FortInventoryServiceComponent.h"

inline static FortInventoryServiceComponent* InventoryService = FortInventoryServiceComponent::Get();

inline static Utils* Util = Utils::Get();

void FortPlayerStateZone::Hook()
{
	Util->HookVTable<AFortPlayerStateAthena>(278, ServerSetInAircraft);
}

void FortPlayerStateZone::ServerSetInAircraft(AFortPlayerStateZone* PlayerState, bool bNewInAircraft)
{
	if (!PlayerState)
		return;

	AFortPlayerControllerAthena* PC = PlayerState->GetOwner()->Cast<AFortPlayerControllerAthena>();
	if (!PC)
		return;

	for (int i = PC->WorldInventory->Inventory.ReplicatedEntries.Num() - 1; i >= 0; i--)
	{
		FFortItemEntry* ItemEntry = &PC->WorldInventory->Inventory.ReplicatedEntries[i];
		if (!ItemEntry || !ItemEntry->ItemDefinition->Cast<UFortWorldItemDefinition>()->bCanBeDropped) continue;
		InventoryService->RemoveItem(PC, ItemEntry->ItemGuid, ItemEntry->Count);
	}
}
