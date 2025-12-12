#pragma once
#include "../../pch.h"

// thanks mariki
struct FStormCircle
{
	FVector Center;
	float Radius;
};

class FortGameStateComponent
{
public:
	static FortGameStateComponent* Get()
	{
		static FortGameStateComponent* Instance = new FortGameStateComponent();
		return Instance;
	}
private:
	std::vector<FStormCircle> StormCircles;
public:
	void SetAircraftTime(const UObject* WorldContext, float DeltaTime);
	void SetGamePhase(const UObject* WorldContext, EAthenaGamePhase GamePhase);
	void SetGamePhaseStep(const UObject* WorldContext, EAthenaGamePhaseStep GamePhaseStep);
	void Tick(const UObject* WorldContext);
	void OnAircraftEnteredDropZone(UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GameStateComponent, AFortAthenaAircraft* FortAthenaAircraft);
	void OnAircraftExitedDropZone(UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GameStateComponent, AFortAthenaAircraft* FortAthenaAircraft);
	void GenerateStormCircles(AFortAthenaMapInfo* MapInfo);
	void SpawnInitialSafeZone(UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GameStateComponent, AFortGameModeAthena* GameMode, AFortGameStateAthena* GameState);
	void StartNewSafeZonePhase(UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GameStateComponent);
};