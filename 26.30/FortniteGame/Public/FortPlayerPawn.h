#pragma once
#include "../../pch.h"

class FortPlayerPawn
{
public:
	static FortPlayerPawn* Get()
	{
		static FortPlayerPawn* Instance = new FortPlayerPawn();
		return Instance;
	}
public:
	static void Hook();
	static void ServerHandlePickupInfo(AFortPlayerPawn* Pawn, AFortPickup* Pickup, FFortPickupRequestInfo& Params_0);
	static void ServerSendZiplineState(AFortPlayerPawn* Pawn, FZiplinePawnState State);

	inline static void (*TeleportPlayerPawnOG)(UObject* Context, FFrame& Stack, void* Ret);
	static void TeleportPlayerPawn(UObject* Context, FFrame& Stack, void* Ret);
};