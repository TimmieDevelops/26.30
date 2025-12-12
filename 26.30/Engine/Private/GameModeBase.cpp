#include "../Public/GameModeBase.h"

inline static Utils* Util = Utils::Get();

void GameModeBase::Hook()
{
	Util->HookVTable<AFortGameModeBR>(229, SpawnDefaultPawnFor);
}

APawn* GameModeBase::SpawnDefaultPawnFor(AGameModeBase* GameMode, AController* NewPlayer, AActor* StartSpot)
{
	APawn* Pawn = GameMode->SpawnDefaultPawnAtTransform(NewPlayer, StartSpot->GetTransform());

	Pawn->NetUpdateFrequency = 100.f;
	Pawn->MinNetUpdateFrequency = 30.f;
	Pawn->SetReplicateMovement(true);
	Pawn->OnRep_ReplicatedMovement();

	return Pawn;
}
