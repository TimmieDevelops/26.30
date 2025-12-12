#pragma once
#include "../../pch.h"

class GameModeBase
{
public:
	static GameModeBase* Get()
	{
		static GameModeBase* Instance = new GameModeBase();
		return Instance;
	}
public:
	static void Hook();
	static APawn* SpawnDefaultPawnFor(AGameModeBase* GameMode, AController* NewPlayer, AActor* StartSpot);
};