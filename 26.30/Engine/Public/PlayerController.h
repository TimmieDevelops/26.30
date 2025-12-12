#pragma once
#include "../../pch.h"

class PlayerController
{
public:
	static PlayerController* Get()
	{
		static PlayerController* Instance = new PlayerController();
		return Instance;
	}
public:
	static void Hook();
	static void SendClientAdjustment(APlayerController* PC);
	static void ServerAcknowledgePossession(APlayerController* PC, APawn* P);
};