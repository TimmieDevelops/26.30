#include "../Public/Iris.h"

#include "../../Engine/Public/NetDriver.h"

inline static NetDriver* NetworkDriver = NetDriver::Get();

Iris::Iris(UNetDriver* InDriver, float DeltaTime) :
	Driver(InDriver),
	DeltaSeconds(DeltaTime)
{
}

void Iris::ServerReplicateActors()
{
	if (Driver->GetReplicationSystem())
	{
		Driver->UpdateReplicationViews();
		NetworkDriver->SendClientMoveAdjustments(Driver);
		FSendUpdateParams Params;
		Params.DeltaSeconds = DeltaSeconds;
		Params.SendPass = EReplicationSystemSendPass::TickFlush;
		Driver->GetReplicationSystem()->PreSendUpdate(Params);
	}
}
