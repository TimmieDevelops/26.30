#include "../Public/NetDriver.h"
#include "../Public/PlayerController.h"

#include "../../Replication/Public/Legacy.h"
#include "../../Replication/Public/Iris.h"

#include "../../FortniteGame/Public/FortGameStateComponent.h"

inline static PlayerController* PC = PlayerController::Get();

inline static Globals* Global = Globals::Get();

inline static FortGameStateComponent* GameStateComponent = FortGameStateComponent::Get();

void NetDriver::Hook()
{
	MH_CreateHook((LPVOID)(GetImageBase() + 0x381B060), TickFlush, (LPVOID*)(&TickFlushOG));
}

void* NetDriver::TickFlush(UNetDriver* Driver, float DeltaSeconds)
{
	if (Driver->ClientConnections.Num() > 0)
	{
		if (!Global->bIris)
		{
			Legacy(Driver, DeltaSeconds).ServerReplicateActors();
		}
		else
		{
			Iris(Driver, DeltaSeconds).ServerReplicateActors();
		}

		GameStateComponent->Tick(Driver->World);
	}

	return TickFlushOG(Driver, DeltaSeconds);
}

void NetDriver::SendClientMoveAdjustments(UNetDriver* Driver)
{
	for (UNetConnection* Connection : Driver->ClientConnections)
	{
		if (!Connection || !Connection->ViewTarget)
			continue;

		if (APlayerController* Controller = Connection->PlayerController)
			PC->SendClientAdjustment(Controller);

		for (UNetConnection* ChildConnection : Connection->Children)
		{
			if (!ChildConnection)
				continue;

			if (APlayerController* Controller = ChildConnection->PlayerController)
				PC->SendClientAdjustment(Controller);
		}
	}
}
