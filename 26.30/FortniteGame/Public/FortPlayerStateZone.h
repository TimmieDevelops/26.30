#pragma once
#include "../../pch.h"

class FortPlayerStateZone
{
public:
	static FortPlayerStateZone* Get()
	{
		static FortPlayerStateZone* Instance = new FortPlayerStateZone();
		return Instance;
	}
public:
	static void Hook();
	static void ServerSetInAircraft(AFortPlayerStateZone* PlayerState, bool bNewInAircraft);
};