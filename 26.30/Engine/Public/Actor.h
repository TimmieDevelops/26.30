#pragma once
#include "../../pch.h"

class Actor
{
public:
	static Actor* Get()
	{
		static Actor* Instance = new Actor();
		return Instance;
	}
private:
	inline static void (*InitalizeActorsOG)(AActor* Actor);
public:
	static void Hook();
	static ENetMode InternalGetNetMode(AActor* Actor);
	static void ForceNetRelevant(AActor* Actor);
	static void InitalizeActors(AActor* Actor);
};