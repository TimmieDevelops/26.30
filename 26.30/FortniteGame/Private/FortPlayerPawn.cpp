#include "../Public/FortPlayerPawn.h"

#include "../../Engine/Public/Actor.h"

inline static Actor* Actors = Actor::Get();

inline static Utils* Util = Utils::Get();

void FortPlayerPawn::Hook()
{
	Util->HookVTable<APlayerPawn_Athena_C>(572, ServerHandlePickupInfo);
}

void FortPlayerPawn::ServerHandlePickupInfo(AFortPlayerPawn* Pawn, AFortPickup* Pickup, FFortPickupRequestInfo& Params_0)
{
	if (!Pawn || !Pickup)
		return;

	if (Pickup->bPickedUp)
		return;
	
	Util->Logger(Info, "ServerHandlePickupInfo", std::format("bReplicates={} bAlwaysRelevant={}", static_cast<int32>(Pickup->bReplicates), static_cast<int32>(Pickup->bAlwaysRelevant)).c_str());

	Pickup->bPickedUp = true;
	Pickup->OnRep_bPickedUp();

	TWeakObjectPtr<AFortPawn> WeakPawn{};
	WeakPawn.ObjectIndex = Pawn->Index;

	Pickup->PickupLocationData.PickupTarget = WeakPawn;
	Pickup->PickupLocationData.ItemOwner = WeakPawn;
	Pickup->PickupLocationData.FlyTime = 0.5f;
	//Pickup->PickupLocationData.LootInitialPosition = (FVector_NetQuantize10&)Params_0.Direction;
	//Pickup->PickupLocationData.LootFinalPosition = (FVector_NetQuantize10&)Params_0.Direction;
	Pickup->PickupLocationData.StartDirection = (FVector_NetQuantizeNormal&)Params_0.Direction;
	Pickup->PickupLocationData.bPlayPickupSound = Params_0.bPlayPickupSound;
	Pickup->PickupLocationData.PickupGuid = Pawn->CurrentWeapon->ItemEntryGuid;
	Pickup->OnRep_PickupLocationData();

	Pawn->IncomingPickups.Add(Pickup);
}
