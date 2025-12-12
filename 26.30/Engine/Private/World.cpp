#include "../Public/World.h"

inline static Globals* Global = Globals::Get();
inline static Utils* Util = Utils::Get();

void World::Hook()
{
	Util->PatchAllNetModes(GetImageBase() + 0x1D7D9E8); // i will slap somebody dick next life
	MH_CreateHook((LPVOID)(GetImageBase() + 0xD40014), InternalGetNetMode, nullptr);
}

ENetMode World::InternalGetNetMode(UWorld* World)
{
	return Global->NetMode;
}
