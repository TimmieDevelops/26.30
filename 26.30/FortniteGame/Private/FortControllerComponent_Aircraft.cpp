#include "../Public/FortControllerComponent_Aircraft.h"

inline static Utils* Util = Utils::Get();

void FortControllerComponent_Aircraft::Hook()
{
	Util->HookVTable<UFortControllerComponent_Aircraft>(165, ServerAttemptAircraftJump);
}

void FortControllerComponent_Aircraft::ServerAttemptAircraftJump(UFortControllerComponent_Aircraft* ControllerComponent, FRotator ClientRotation)
{
	if (!ControllerComponent)
		return;

	AFortGameModeAthena* GameMode = UWorld::GetWorld()->AuthorityGameMode->Cast<AFortGameModeAthena>();
	if (!GameMode)
		return;

	AFortPlayerControllerAthena* PC = ControllerComponent->GetOwner()->Cast<AFortPlayerControllerAthena>();
	if (!PC)
		return;

	if (PC->IsInAircraft())
	{
		GameMode->RestartPlayer(PC);
		PC->PlayerState->Cast<AFortPlayerStateAthena>()->bInAircraft = false;
		PC->GetPlayerPawn()->BeginSkydiving(true);
	}
}
