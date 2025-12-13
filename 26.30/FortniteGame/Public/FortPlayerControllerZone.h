#pragma once
#include "../../pch.h"

class FortPlayerControllerZone
{
public:
	static FortPlayerControllerZone* Get()
	{
		static FortPlayerControllerZone* Instance = new FortPlayerControllerZone();
		return Instance;
	}
private:
	inline static void (*ClientOnPawnDiedOG)(AFortPlayerControllerZone* PC, FFortPlayerDeathReport& DeathReport);
public:
	static void Hook();
	static void ClientOnPawnDied(AFortPlayerControllerZone* PC, FFortPlayerDeathReport& DeathReport);
};