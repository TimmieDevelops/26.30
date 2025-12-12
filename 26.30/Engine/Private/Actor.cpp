#include "../Public/Actor.h"

inline static Globals* Global = Globals::Get();
inline static Utils* Util = Utils::Get();

void Actor::Hook()
{
	MH_CreateHook((LPVOID)(GetImageBase() + 0xDE39C8), InternalGetNetMode, nullptr);
	MH_CreateHook((LPVOID)(GetImageBase() + 0xD5FEB4), InitalizeActors, (LPVOID*)(&InitalizeActorsOG));
}

ENetMode Actor::InternalGetNetMode(AActor* Actor)
{
	return Global->NetMode;
}

// https://github.com/EpicGames/UnrealEngine/blob/7f25d6a85a0ce24625c217ed7155f53a39e36a9c/Engine/Source/Runtime/Engine/Private/Actor.cpp#L3230
void Actor::ForceNetRelevant(AActor* Actor)
{
	if (Actor->RemoteRole == ENetRole::ROLE_None)
	{
		if (!Actor->bReplicates)
			Actor->bReplicates = true;
		Actor->bAlwaysRelevant = true;
		if (Actor->NetUpdateFrequency == 0.f)
			Actor->NetUpdateFrequency = 0.1f;
	}

	Actor->ForceNetUpdate();
}

void Actor::InitalizeActors(AActor* Actor)
{
	InitalizeActorsOG(Actor);
	Actor->bAlwaysRelevant = true;
}
