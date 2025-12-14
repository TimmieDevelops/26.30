#include "../Public/FortPlayerPawn.h"

#include "../../Engine/Public/Actor.h"

inline static Actor* Actors = Actor::Get();

inline static Utils* Util = Utils::Get();

void FortPlayerPawn::Hook()
{
	Util->HookVTable<APlayerPawn_Athena_C>(572, ServerHandlePickupInfo);
	Util->HookVTable<APlayerPawn_Athena_C>(0x24B, ServerSendZiplineState);
	Util->CreateFuncHook<AFortPlayerPawn>("/Script/FortniteGame.FortPlayerPawn", "TeleportPlayerPawn", TeleportPlayerPawn, (void**)&TeleportPlayerPawnOG);
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

void FortPlayerPawn::ServerSendZiplineState(AFortPlayerPawn* Pawn, FZiplinePawnState State)
{
	if (!Pawn) 
	{
		return;
	}

	Pawn->ZiplineState = State;
	auto Zipline = Pawn->GetActiveZipline();
	((void (*)(AFortPlayerPawn*))(InSDKUtils::GetImageBase() + 0x87D570C))(Pawn);
	if (State.bJumped) 
	{
		auto Velocity = Pawn->CharacterMovement->Velocity;
		auto VelocityX = Velocity.X * -0.5f;
		auto VelocityY = Velocity.Y * -0.5f;
		auto VelocityZ = Velocity.Z * -0.5f;
		Pawn->LaunchCharacterJump({ VelocityX >= -750 ? fminf(VelocityX, 750) : -750, VelocityY >= -750 ? fminf(VelocityY, 750) : -750, 1200 }, false, false, true, true);
	}
	static auto ZipLineClass = Utils::StaticLoadObject<UClass>("/Ascender/Gameplay/Ascender/B_Athena_Zipline_Ascender.B_Athena_Zipline_Ascender_C");
	if (auto Ascender = Zipline->Cast<AFortAscenderZipline>(ZipLineClass)) 
	{
		TWeakObjectPtr<AFortPlayerPawn> PawnUsingHandle{};
		PawnUsingHandle.ObjectIndex = -1;
		Ascender->PawnUsingHandle = PawnUsingHandle;
		Ascender->OnRep_PawnUsingHandle();
	}
}

void FortPlayerPawn::TeleportPlayerPawn(UObject* Context, FFrame& Stack, void* Ret)
{
	UObject* WorldContextObject = nullptr;
	AFortPlayerPawn* PlayerPawn = nullptr;
	FVector DestLocation;
	FRotator DestRotation;
	bool bIgnoreCollision = false;
	bool bIgnoreSupplementalKillVolumeSweep = false;
	Stack.StepCompiledIn(&WorldContextObject);
	Stack.StepCompiledIn(&PlayerPawn);
	Stack.StepCompiledIn(&DestLocation);
	Stack.StepCompiledIn(&DestRotation);
	Stack.StepCompiledIn(&bIgnoreCollision);
	Stack.StepCompiledIn(&bIgnoreSupplementalKillVolumeSweep);
	bool bS = false;
	if (PlayerPawn)
	{
		bS = PlayerPawn->K2_TeleportTo(DestLocation, DestRotation);
	}
	*static_cast<bool*>(Ret) = true;
}
