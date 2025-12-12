#include "../Public/FortLootPackage.h"

#include "../../Engine/Public/Actor.h"

inline static Actor* Actors = Actor::Get();

inline static Utils* Util = Utils::Get();
inline static Globals* Global = Globals::Get();

void FortLootPackage::Init(UWorld* World, AFortGameStateAthena* GameState)
{
	TArray<AActor*> Warmup;
	UGameplayStatics::GetAllActorsOfClass(World, UObject::FindObject<UBlueprintGeneratedClass>("BlueprintGeneratedClass Tiered_Athena_FloorLoot_Warmup.Tiered_Athena_FloorLoot_Warmup_C"), &Warmup);

	for (AActor* Actor : Warmup)
	{
		if (!Actor)
			continue;

		Actor->K2_DestroyActor();
	}

	Warmup.Free();

	UGameplayStatics::GetAllActorsOfClass(World, UObject::FindObject<UBlueprintGeneratedClass>("BlueprintGeneratedClass Tiered_Athena_FloorLoot_01.Tiered_Athena_FloorLoot_01_C"), &Warmup);

	for (AActor* Actor : Warmup)
	{
		if (!Actor)
			continue;

		Actor->K2_DestroyActor();
	}

	Warmup.Free();
}

std::vector<FFortItemEntry*> FortLootPackage::PickLootDrops(FName TierGroupName, int WorldLevel)
{
	std::vector<FFortItemEntry*> Results;

	static UDataTable* LootPackagesData = Util->StaticLoadObject<UDataTable>("/Game/Items/DataTables/AthenaLootPackages_Client");
	static UDataTable* LootTierData = Util->StaticLoadObject<UDataTable>("/Game/Items/DataTables/AthenaLootTierData_Client");
	if (!LootPackagesData || !LootTierData)
	{
		Util->Logger(Info, "PickLootDrops", "NuLL!");
		return Results;
	}

	std::vector<FFortLootTierData*> LootTiers;
	for (auto& Row : LootTierData->RowMap)
	{
		FFortLootTierData* Value = reinterpret_cast<FFortLootTierData*>(Row.Value());
		if (!Value)
			continue;

		if (Value->TierGroup != TierGroupName)
			continue;

		if (Value->Weight <= 0.f)
			continue;

		if (Value->MinWorldLevel != -1 &&
			WorldLevel < Value->MinWorldLevel)
			continue;

		if (Value->MaxWorldLevel != -1 &&
			WorldLevel > Value->MaxWorldLevel)
			continue;

		LootTiers.push_back(Value);
	}

	if (LootTiers.empty())
		return Results;

	FFortLootTierData* LootTier = SelectLootTier(LootTiers);
	if (!LootTier)
		return Results;

	std::vector<FFortLootPackageData*> LootPackages;
	for (auto& Row : LootPackagesData->RowMap)
	{
		FFortLootPackageData* Value = reinterpret_cast<FFortLootPackageData*>(Row.Value());
		if (!Value)
			continue;

		if (Value->LootPackageID != LootTier->LootPackage)
			continue;

		if (Value->Weight <= 0.f)
			continue;

		if (Value->MinWorldLevel != -1 &&
			WorldLevel < Value->MinWorldLevel)
			continue;

		if (Value->MaxWorldLevel != -1 &&
			WorldLevel > Value->MaxWorldLevel)
			continue;

		LootPackages.push_back(Value);
	}

	if (LootPackages.empty())
		return Results;

	for (float Drops = 0; Drops < LootTier->NumLootPackageDrops; ++Drops)
	{
		FFortLootPackageData* Package = LootPackages[Drops];
		if (!Package)
			continue;

		std::vector<FFortLootPackageData*> NewLootPackages;
		if (Package->ItemDefinition.ObjectID.AssetPath.AssetName.ToString() == "None")
		{
			for (auto& Row : LootPackagesData->RowMap)
			{
				FFortLootPackageData* Value = reinterpret_cast<FFortLootPackageData*>(Row.Value());
				if (!Value)
					continue;

				if (Value->LootPackageID.ToString() != Package->LootPackageCall.ToString())
					continue;

				if (Value->MinWorldLevel != -1 &&
					WorldLevel < Value->MinWorldLevel)
					continue;

				if (Value->MaxWorldLevel != -1 &&
					WorldLevel > Value->MaxWorldLevel)
					continue;

				NewLootPackages.push_back(Value);
			}
		}
		else
		{
			NewLootPackages.push_back(Package);
		}

		FFortLootPackageData* NewPackage = SelectLootPackage(NewLootPackages);
		if (!NewPackage)
			continue;

		UFortItemDefinition* ItemDefinition = Util->StaticLoadObject<UFortItemDefinition>(UKismetStringLibrary::Conv_NameToString(NewPackage->ItemDefinition.ObjectID.AssetPath.PackageName).ToString());
		if (!ItemDefinition)
			continue;

		FFortItemEntry* ItemEntry = new FFortItemEntry();
		ItemEntry->ItemDefinition = ItemDefinition;
		ItemEntry->Count = NewPackage->Count;

		Results.push_back(ItemEntry);
	}

	return Results;
}

AFortPickup* FortLootPackage::SpawnPickup(FVector Location, UFortItemDefinition* ItemDefinition, int Count, int LoadedAmmo, EFortPickupSourceTypeFlag TypeFlag, EFortPickupSpawnSource SpawnSource, AFortPawn* Pawn)
{
	AFortPickup* Pickup = Util->SpawnActor<AFortPickup>(Location);

	Pickup->bRandomRotation = true;
	
	FFortItemEntry& ItemEntry = Pickup->PrimaryPickupItemEntry;
	ItemEntry.ItemDefinition = ItemDefinition;
	ItemEntry.Count = Count;
	ItemEntry.LoadedAmmo = LoadedAmmo;
	Pickup->OnRep_PrimaryPickupItemEntry();

	if (Pawn)
	{
		TWeakObjectPtr<AFortPawn> WeakPawn{};
		WeakPawn.ObjectIndex = Pawn->Index;
		Pickup->PawnWhoDroppedPickup = WeakPawn;
	}

	Pickup->TossPickup(Location, Pawn, -1, true, true, TypeFlag, SpawnSource);

	Pickup->SetReplicateMovement(true);
	Pickup->MovementComponent = UGameplayStatics::SpawnObject(UProjectileMovementComponent::StaticClass(), Pickup)->Cast<UFortProjectileMovementComponent>();

	if (TypeFlag == EFortPickupSourceTypeFlag::Container)
	{
		Pickup->bTossedFromContainer = true;
		Pickup->OnRep_TossedFromContainer();
	}

	return Pickup;
}

FVector FortLootPackage::GetLocationSpawnDrop(AActor* Actor)
{
	float X = UKismetMathLibrary::RandomFloatInRange(-80.f, 80.f);
	float Y = UKismetMathLibrary::RandomFloatInRange(-80.f, 80.f);
	float Z = 40.f;

	FVector SpawnLocation = Actor->K2_GetActorLocation() + FVector(X, Y, Z);
	return SpawnLocation;
}

FFortLootTierData* FortLootPackage::SelectLootTier(const std::vector<FFortLootTierData*>& OutLootTiers)
{
	if (OutLootTiers.empty())
	{
		Util->Logger(Info, "FortLootPackage::SelectLootTier", "OutLootTiers is empty!");
		return nullptr;
	}

	float TotalWeight = 0.f;

	for (FFortLootTierData* Row : OutLootTiers)
	{
		if (!Row)
			continue;

		TotalWeight += Row->Weight;
	}

	float R = UKismetMathLibrary::RandomFloatInRange(0.f, TotalWeight);
	float Acc = 0.f;

	for (FFortLootTierData* Row : OutLootTiers)
	{
		if (!Row)
			continue;

		Acc += Row->Weight;

		if (R <= Acc)
			return Row;
	}

	return nullptr;
}

FFortLootPackageData* FortLootPackage::SelectLootPackage(const std::vector<FFortLootPackageData*>& OutLootPackages)
{
	if (OutLootPackages.empty())
	{
		Util->Logger(Info, "FortLootPackage::SelectLootPackage", "OutLootPackages is empty!");
		return nullptr;
	}

	float TotalWeight = 0.f;
	for (FFortLootPackageData* Row : OutLootPackages)
		TotalWeight += Row->Weight;

	float R = UKismetMathLibrary::RandomFloatInRange(0.f, TotalWeight);
	float Acc = 0.f;

	for (FFortLootPackageData* Row : OutLootPackages)
	{
		Acc += Row->Weight;
		if (R <= Acc)
			return Row;
	}

	return nullptr;
}
