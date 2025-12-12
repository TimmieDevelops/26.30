#include "../Public/FortInventoryServiceComponent.h"
#include "../Public/FortLootPackage.h"

inline static FortLootPackage* LootPackage = FortLootPackage::Get();

inline static Utils* Util = Utils::Get();

void FortInventoryServiceComponent::Hook()
{
	MH_CreateHook((LPVOID)(GetImageBase() + 0x78246BC), SetupInventoryServiceComponent, nullptr);
}

void FortInventoryServiceComponent::SetupInventoryServiceComponent(UFortControllerComponent_InventoryService* InventoryService)
{
	//Util->Logger(ELogType::Info, "FortInventoryServiceComponent::SetupInventoryServiceComponent", InventoryService->GetFullName());
	if (!InventoryService)
		return;

	AFortPlayerControllerAthena* PC = InventoryService->GetOwner()->Cast<AFortPlayerControllerAthena>();
	if (!PC)
		return;

	AFortGameModeAthena* GameMode = UWorld::GetWorld()->AuthorityGameMode->Cast<AFortGameModeAthena>();
	if (!GameMode)
		return;

	for (int i = 0; i < GameMode->StartingItems.Num(); i++)
	{
		if (GameMode->StartingItems[i].Count > 0)
		{
			if (GameMode->StartingItems[i].Item->GetName().contains("Smart")) continue;
			GiveItem(PC, GameMode->StartingItems[i].Item, GameMode->StartingItems[i].Count); //E 
		}
	}

	GiveItem(PC, PC->CosmeticLoadoutPC.Pickaxe->WeaponDefinition, 1);
	Update(PC);
}

UFortWorldItem* FortInventoryServiceComponent::GiveItem(AFortPlayerControllerAthena* PC, UFortItemDefinition* ItemDefinition, int Count, int LoadedAmmo)
{
	if (!PC || !ItemDefinition)
		return nullptr;

	UFortWorldItem* NewInstance = ItemDefinition->CreateTemporaryItemInstanceBP(Count, 1)->Cast<UFortWorldItem>();

	FFortItemEntry& ItemEntry = NewInstance->ItemEntry;
	ItemEntry.Count = Count;
	ItemEntry.LoadedAmmo = LoadedAmmo;
	NewInstance->SetOwningControllerForTemporaryItem(PC);

	PC->WorldInventory->Inventory.ReplicatedEntries.Add(ItemEntry);
	PC->WorldInventory->Inventory.ItemInstances.Add(NewInstance);

	return NewInstance;
}

void FortInventoryServiceComponent::Update(AFortPlayerControllerAthena* PC, FFortItemEntry* ItemEntry)
{
	if (!PC)
		return;

	PC->HandleWorldInventoryLocalUpdate();
	PC->WorldInventory->HandleInventoryLocalUpdate();
	PC->WorldInventory->bRequiresLocalUpdate = true;
	PC->WorldInventory->ForceNetUpdate();
	if (!ItemEntry)
		PC->WorldInventory->Inventory.MarkArrayDirty();
	else
		PC->WorldInventory->Inventory.MarkItemDirty(ItemEntry);
}

FFortItemEntry* FortInventoryServiceComponent::FindEntry(AFortPlayerControllerAthena* PC, FGuid& ItemGuid)
{
	if (!PC)
		return nullptr;

	for (FFortItemEntry& ItemEntry : PC->WorldInventory->Inventory.ReplicatedEntries)
	{
		if (ItemEntry.ItemGuid == ItemGuid)
			return &ItemEntry;
	}

	for (UFortWorldItem* ItemInstance : PC->WorldInventory->Inventory.ItemInstances)
	{
		if (ItemInstance && ItemInstance->GetItemGuid() == ItemGuid)
			return &ItemInstance->ItemEntry;
	}

	return nullptr;
}

FFortItemEntry* FortInventoryServiceComponent::FindEntry(AFortPlayerControllerAthena* PC, UFortItemDefinition* ItemDefinition)
{
	if (!PC || !ItemDefinition)
		return nullptr;

	for (FFortItemEntry& ItemEntry : PC->WorldInventory->Inventory.ReplicatedEntries)
	{
		if (ItemEntry.ItemDefinition == ItemDefinition)
			return &ItemEntry;
	}

	for (UFortWorldItem* ItemInstance : PC->WorldInventory->Inventory.ItemInstances)
	{
		if (ItemInstance && ItemInstance->GetItemDefinitionBP() == ItemDefinition)
			return &ItemInstance->ItemEntry;
	}

	return nullptr;
}

void FortInventoryServiceComponent::AddItem(AFortPlayerControllerAthena* PC, UFortItemDefinition* ItemDefinition, int Count, int LoadedAmmo)
{
	if (!PC || !ItemDefinition)
		return;

	if (Count <= 0)
		return;

	if (FFortItemEntry* Exist = FindEntry(PC, ItemDefinition))
	{
		if (!Exist)
			return;

		float MaxStackable = Util->ReadCurve(ItemDefinition->MaxStackSize);
		int32 SpaceLeft = MaxStackable - Exist->Count;

		if (SpaceLeft <= 0)
		{
			LootPackage->SpawnPickup(LootPackage->GetLocationSpawnDrop(PC->Pawn), ItemDefinition, Count, 0, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::LootDrop, PC->MyFortPawn);
			return;
		}

		int Addable = UKismetMathLibrary::min_0(Count, SpaceLeft);
		Exist->Count += Addable;
		Count -= Addable;

		if (Count <= 0)
		{
			Update(PC, Exist);
			return;
		}

		LootPackage->SpawnPickup(LootPackage->GetLocationSpawnDrop(PC->Pawn), ItemDefinition, Count, 0, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::LootDrop, PC->MyFortPawn);

		Update(PC, Exist);
		return;
	}

	GiveItem(PC, ItemDefinition, Count, LoadedAmmo);
	Update(PC);
}

void FortInventoryServiceComponent::RemoveItem(AFortPlayerControllerAthena* PC, FGuid& ItemGuid, int Count)
{
	if (!PC)
		return;

	if (Count <= 0)
		return;

	FFortItemEntry* Entry = FindEntry(PC, ItemGuid);
	if (!Entry)
		return;

	int32 RemoveAmount = (Entry->Count < Count) ? Entry->Count : Count;

	Entry->Count -= RemoveAmount;

	if (Entry->Count <= 0)
	{
		for (int i = 0; i < PC->WorldInventory->Inventory.ItemInstances.Num(); i++)
		{
			UFortWorldItem* ItemInstance = PC->WorldInventory->Inventory.ItemInstances[i];
			if (!ItemInstance)
				continue;

			if (ItemInstance->GetItemGuid() == ItemGuid)
			{
				PC->WorldInventory->Inventory.ItemInstances.Remove(i);
				break;
			}
		}

		for (int i = 0; i < PC->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
		{
			FFortItemEntry* ItemEntry = &PC->WorldInventory->Inventory.ReplicatedEntries[i];
			if (!ItemEntry)
				continue;

			if (ItemEntry->ItemGuid == ItemGuid)
			{
				PC->WorldInventory->Inventory.ReplicatedEntries.Remove(i);
				break;
			}
		}
	}

	Update(PC, Entry);
}
