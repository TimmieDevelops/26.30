#include "../Public/FortPlayerControllerZone.h"
#include "../Public/FortInventoryServiceComponent.h"

inline static FortInventoryServiceComponent* InventoryService = FortInventoryServiceComponent::Get();

inline static Utils* Util = Utils::Get();

void FortPlayerControllerZone::Hook()
{
	Util->HookVTable<AFortPlayerControllerAthena>(1123, ClientOnPawnDied, (void**)(&ClientOnPawnDiedOG));
}

void FortPlayerControllerZone::ClientOnPawnDied(AFortPlayerControllerZone* PC, FFortPlayerDeathReport& DeathReport)
{
	ClientOnPawnDiedOG(PC, DeathReport);

	if (!PC)
		return;

	AFortGameModeAthena* GameMode = UWorld::GetWorld()->AuthorityGameMode->Cast<AFortGameModeAthena>();
	if (!GameMode)
		return;

	AFortPlayerControllerAthena* Controller = PC->Cast<AFortPlayerControllerAthena>();
	AFortPlayerStateAthena* PlayerState = PC->PlayerState->Cast<AFortPlayerStateAthena>();
	AFortPlayerPawnAthena* PlayerPawn = PC->GetPlayerPawn()->Cast<AFortPlayerPawnAthena>();
	if (!Controller)
		return;

	AFortPlayerStateAthena* KillerPlayerState = DeathReport.KillerPlayerState->Cast<AFortPlayerStateAthena>();
	AFortPlayerControllerAthena* KillerController = KillerPlayerState->GetOwner()->Cast<AFortPlayerControllerAthena>();
	AFortPlayerPawnAthena* KillerPlayerPawn = KillerController->GetPlayerPawn()->Cast<AFortPlayerPawnAthena>();
	if (!KillerController)
		return;

	TWeakObjectPtr<AActor> WeakPlayerState{};
	WeakPlayerState.ObjectIndex = KillerPlayerState->Index ? KillerPlayerState->Index : PlayerState->Index;

	FDeathInfo DeathInfo;
	DeathInfo.FinisherOrDowner = WeakPlayerState;
	DeathInfo.Downer = WeakPlayerState;
	DeathInfo.bDBNO = PlayerPawn->IsDBNO();
	DeathInfo.DeathCause = PlayerState->ToDeathCause(DeathReport.Tags, DeathInfo.bDBNO);
	DeathInfo.Distance = PlayerPawn->GetDistanceTo(KillerPlayerPawn);
	DeathInfo.DeathLocation = PlayerPawn->K2_GetActorLocation();
	DeathInfo.bInitialized = true;
	DeathInfo.DeathTags = DeathReport.Tags;

	if (PlayerPawn && KillerPlayerState && KillerPlayerState != PlayerState)
	{
		KillerPlayerState->KillScore++;
		KillerPlayerState->TeamKillScore++;
		KillerPlayerState->OnRep_Kills();
		KillerPlayerState->ClientReportKill(PlayerState);
		KillerPlayerState->ClientReportTeamKill(KillerPlayerState->TeamKillScore);
	}

	for (int i = PC->WorldInventory->Inventory.ReplicatedEntries.Num() - 1; i >= 0; i--)
	{
		FFortItemEntry* ItemEntry = &PC->WorldInventory->Inventory.ReplicatedEntries[i];
		if (!ItemEntry || !ItemEntry->ItemDefinition->Cast<UFortWorldItemDefinition>()->bCanBeDropped) continue;
		InventoryService->RemoveItem(Controller, ItemEntry->ItemGuid, ItemEntry->Count);
	}

	Util->Logger(Info, "ClientOnPawnDied", "DIED");
	GameMode->RemoveFromAlivePlayers(Controller, PlayerState == KillerPlayerState ? nullptr : KillerPlayerState, KillerPlayerPawn, nullptr, DeathInfo.DeathCause, 0);
}
