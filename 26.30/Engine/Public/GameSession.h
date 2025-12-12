#pragma once
#include "../../pch.h"

class GameSession
{
public:
	static GameSession* Get()
	{
		static GameSession* Instance = new GameSession();
		return Instance;
	}
public:
	static void Hook();
	static bool KickPlayer(AGameSession* GameSession, APlayerController* KickedPlayer, const FText& KickReason);
};