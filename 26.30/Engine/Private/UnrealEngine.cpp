#include "../Public/UnrealEngine.h"

inline static Globals* Global = Globals::Get();

void UnrealEngine::Hook()
{
    MH_CreateHook((LPVOID)(GetImageBase() + 0x15A64B4), GetMaxTickRate, nullptr);
}

float UnrealEngine::GetMaxTickRate()
{
    return Global->MaxTickRate;
}
