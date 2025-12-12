#pragma once
#include "../../pch.h"

class FortControllerComponent_Aircraft
{
public:
	static FortControllerComponent_Aircraft* Get()
	{
		static FortControllerComponent_Aircraft* Instance = new FortControllerComponent_Aircraft();
		return Instance;
	}
public:
	static void Hook();
	static void ServerAttemptAircraftJump(UFortControllerComponent_Aircraft* ControllerComponent, FRotator ClientRotation);
};