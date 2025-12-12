#include "../Public/FortGameStateComponent.h"

#include "../../Server/SafeZone.h"

inline static Utils* Util = Utils::Get();

void FortGameStateComponent::SetAircraftTime(const UObject* WorldContext, float DeltaTime)
{
	UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GameStateComponent = UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Get(WorldContext);
	float TimeSeconds = UGameplayStatics::GetTimeSeconds(WorldContext);
	
	GameStateComponent->WarmupCountdownEndTime = TimeSeconds + DeltaTime;
	GameStateComponent->WarmupCountdownDuration = DeltaTime;
	
	GameStateComponent->WarmupCountdownStartTime = TimeSeconds;
	GameStateComponent->WarmupEarlyCountdownDuration = DeltaTime;
	GameStateComponent->OnRep_WarmupCountdownEndTime();
}

void FortGameStateComponent::SetGamePhase(const UObject* WorldContext, EAthenaGamePhase GamePhase)
{
	UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GameStateComponent = UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Get(WorldContext);

	EAthenaGamePhase OldGamePhase = GameStateComponent->GamePhase; //GameStateComponent->GetGamePhase();
	GameStateComponent->GamePhase = GamePhase;
	GameStateComponent->OnRep_GamePhase(OldGamePhase);
}

void FortGameStateComponent::SetGamePhaseStep(const UObject* WorldContext, EAthenaGamePhaseStep GamePhaseStep)
{
	UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GameStateComponent = UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Get(WorldContext);

	EAthenaGamePhaseStep OldGamePhaseStep = GameStateComponent->GamePhaseStep; //GameStateComponent->GetGamePhaseStep();
	GameStateComponent->GamePhaseStep = GamePhaseStep;
	GameStateComponent->HandleGamePhaseStepChanged(OldGamePhaseStep); // iG?
}

void FortGameStateComponent::Tick(const UObject* WorldContext)
{
	UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GameStateComponent = UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Get(WorldContext);
	EAthenaGamePhase GamePhase = GameStateComponent->GamePhase; //GameStateComponent->GetGamePhase();
	EAthenaGamePhaseStep GamePhaseStep = GameStateComponent->GamePhaseStep; //GameStateComponent->GetGamePhaseStep();
	float TimeSeconds = UGameplayStatics::GetTimeSeconds(WorldContext);
	AFortGameModeAthena* GameMode = UWorld::GetWorld()->AuthorityGameMode->Cast<AFortGameModeAthena>();
	AFortGameStateAthena* GameState = GameMode->GameState->Cast<AFortGameStateAthena>();

	static bool bIsReady = false;
	static bool bHasAircraftStarted = false;
	static bool bHasFinishedAircraft = false;
	static bool bHasStormStarted = false;

	if (!bIsReady && GamePhase == EAthenaGamePhase::Warmup && GamePhaseStep == EAthenaGamePhaseStep::Warmup && GameStateComponent->GetWarmupCountdownEndTime() - TimeSeconds <= 10.f)
	{
		bIsReady = true;
		SetGamePhaseStep(WorldContext, EAthenaGamePhaseStep::GetReady);
	}
	else if (bIsReady && GamePhase == EAthenaGamePhase::Warmup && GamePhaseStep == EAthenaGamePhaseStep::GetReady && GameStateComponent->GetWarmupCountdownEndTime() - TimeSeconds <= 0.f)
	{
		SetGamePhase(WorldContext, EAthenaGamePhase::Aircraft);
		SetGamePhaseStep(WorldContext, EAthenaGamePhaseStep::BusLocked);
	}
	else if (bIsReady && !bHasAircraftStarted && GamePhase == EAthenaGamePhase::Aircraft && GamePhaseStep == EAthenaGamePhaseStep::BusLocked)
	{
		bHasAircraftStarted = true;

		AFortAthenaAircraft* Aircraft = GameStateComponent->Aircrafts_GameMode[0].Get();
		auto& FlightInfo = Aircraft->FlightInfo;

		Aircraft->FlightElapsedTime = 0;

		Aircraft->DropStartTime = TimeSeconds + FlightInfo.TimeTillDropStart;
		Aircraft->DropEndTime = TimeSeconds + FlightInfo.TimeTillDropEnd;

		Aircraft->FlightStartTime = TimeSeconds;
		Aircraft->FlightEndTime = TimeSeconds + FlightInfo.TimeTillFlightEnd;

		Aircraft->ReplicatedFlightTimestamp = TimeSeconds;

		GameStateComponent->bAircraftIsLocked = true;

		GameStateComponent->AircraftStartTime = TimeSeconds;

		OnAircraftEnteredDropZone(GameStateComponent, Aircraft);
	}
	else if (bHasAircraftStarted && GamePhase == EAthenaGamePhase::Aircraft && GamePhaseStep == EAthenaGamePhaseStep::BusLocked && GameStateComponent->Aircrafts_GameMode[0].Get()->DropStartTime - TimeSeconds <= 0.f)
	{
		AFortAthenaAircraft* Aircraft = GameStateComponent->Aircrafts_GameMode[0].Get();
		/*float DropStartTime = Aircraft->DropStartTime - TimeSeconds;
		float FlightEndTime = Aircraft->FlightEndTime - TimeSeconds;
		float FlightStartTime = Aircraft->FlightStartTime - TimeSeconds;
		float DropEndTime = Aircraft->DropEndTime - TimeSeconds; //E
		Util->Logger(Info, "FortGameStateComponent::Tick", std::format("DropStartTime={} FlightEndTime={} FlightStartTime={} DropEndTime={}", DropStartTime, FlightEndTime, FlightStartTime, DropEndTime).c_str());*/

		GameStateComponent->bAircraftIsLocked = false; // we full box pongo
		SetGamePhaseStep(WorldContext, EAthenaGamePhaseStep::BusFlying);
	}
	else if (bHasAircraftStarted && GamePhase == EAthenaGamePhase::Aircraft && GamePhaseStep == EAthenaGamePhaseStep::BusFlying && GameStateComponent->Aircrafts_GameMode[0].Get()->DropEndTime - TimeSeconds <= 0.f)
	{
		AFortAthenaAircraft* Aircraft = GameStateComponent->Aircrafts_GameMode[0].Get();
		OnAircraftExitedDropZone(GameStateComponent, Aircraft);
		GameStateComponent->SafeZonesStartTime = TimeSeconds + 60.f; //GameStateComponent->SetSafeZonesStartTime(TimeSeconds + 60.f);
		SetGamePhase(WorldContext, EAthenaGamePhase::SafeZones);
		SetGamePhaseStep(WorldContext, EAthenaGamePhaseStep::StormForming);
	}
	else if (bHasAircraftStarted && !bHasFinishedAircraft && GamePhase == EAthenaGamePhase::SafeZones && GamePhaseStep == EAthenaGamePhaseStep::StormForming && GameStateComponent->Aircrafts_GameMode[0].Get()->FlightEndTime - TimeSeconds <= 0.f)
	{
		bHasFinishedAircraft = true;

		AFortAthenaAircraft* Aircraft = GameStateComponent->Aircrafts_GameMode[0].Get();
		if (Aircraft)
			Aircraft->K2_DestroyActor();

		GameStateComponent->Aircrafts_GameMode.Free();
		GameStateComponent->Aircrafts_GameState.Free();
	}
	else if (bHasAircraftStarted && GamePhase == EAthenaGamePhase::SafeZones && GamePhaseStep == EAthenaGamePhaseStep::StormForming && GameStateComponent->SafeZonesStartTime - TimeSeconds <= 0.f)
	{
		SpawnInitialSafeZone(GameStateComponent, GameMode, GameState);
		SetGamePhaseStep(WorldContext, EAthenaGamePhaseStep::StormShrinking);
		GameStateComponent->SafeZoneIndicator->OnSafeZoneStateChange(EFortSafeZoneState::Shrinking, false);
		StartNewSafeZonePhase(GameStateComponent);
		bHasStormStarted = true;
	}
	else if (bHasStormStarted)
	{
		if (GamePhaseStep == EAthenaGamePhaseStep::StormShrinking && GameStateComponent->SafeZoneIndicator->SafeZoneFinishShrinkTime /*GameStateComponent->SafeZoneIndicator->GetSafeZoneFinishShrinkTime()*/ - TimeSeconds <= 0.f)
		{
			SetGamePhaseStep(WorldContext, EAthenaGamePhaseStep::StormHolding);
			GameStateComponent->SafeZoneIndicator->OnSafeZoneStateChange(EFortSafeZoneState::Holding, false);
			StartNewSafeZonePhase(GameStateComponent);
		}

		if (GamePhaseStep == EAthenaGamePhaseStep::StormHolding && GameStateComponent->SafeZoneIndicator->SafeZoneStartShrinkTime /*GameStateComponent->SafeZoneIndicator->GetSafeZoneStartShrinkTime()*/ - TimeSeconds <= 0.f)
		{
			SetGamePhaseStep(WorldContext, EAthenaGamePhaseStep::StormShrinking);
			//GameStateComponent->SafeZoneIndicator->OnSafeZoneStateChange(EFortSafeZoneState::Shrinking, false);
			//StartNewSafeZonePhase(GameStateComponent);
		}
	}

	for (AFortPlayerControllerAthena* PC : GameMode->AlivePlayers)
	{
		if (PC->Pawn)
		{
			bool bInZone = GameStateComponent->IsInCurrentSafeZone(PC->Pawn->K2_GetActorLocation(), false);
			Util->Logger(Info, "Tick", std::format("bInZone={}", bInZone).c_str());
		}
	}
}

void FortGameStateComponent::OnAircraftEnteredDropZone(UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GameStateComponent, AFortAthenaAircraft* FortAthenaAircraft)
{
	AFortGameModeAthena* GameMode = UWorld::GetWorld()->AuthorityGameMode->Cast<AFortGameModeAthena>();

	for (AFortPlayerControllerAthena* PC : GameMode->AlivePlayers)
	{
		PC->PlayerState->Cast<AFortPlayerStateAthena>()->bInAircraft = true;
		PC->ClientActivateSlot(EFortQuickBars::Primary, 0, 0.f, true, true);
		if (PC->Pawn) PC->Pawn->K2_DestroyActor();
		PC->Reset();
	}
}

void FortGameStateComponent::OnAircraftExitedDropZone(UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GameStateComponent, AFortAthenaAircraft* FortAthenaAircraft)
{
	AFortGameModeAthena* GameMode = UWorld::GetWorld()->AuthorityGameMode->Cast<AFortGameModeAthena>();

	for (AFortPlayerControllerAthena* PC : GameMode->AlivePlayers)
	{
		if (!PC) continue;
		PC->GetAircraftComponent()->ServerAttemptAircraftJump({});
	}
}

void FortGameStateComponent::GenerateStormCircles(AFortAthenaMapInfo* MapInfo)
{
	if (StormCircles.size() > 0)
		return;

	StormCircles.clear();

	auto FRandSeeded = [&]() { return (float)rand() / 32767.f; };

	float Radii[13] = { 150000, 120000, 95000, 70000, 55000, 32500, 20000, 10000, 5000, 2500, 1650, 1090, 0 };

	FVector FirstCenter = MapInfo->GetMapCenter();
	StormCircles.push_back(FStormCircle{ FirstCenter, Radii[0] });

	float DirAngle = FRandSeeded() * 2.f * UKismetMathLibrary::GetPI();
	float DirSin, DirCos;
	SinCos(&DirSin, &DirCos, DirAngle);
	FVector2D Dir(DirCos, DirSin);
	for (int i = 1; i < 7; ++i)
	{
		FVector RefCenter = StormCircles[i - 1].Center;
		float RefRadius = StormCircles[i - 1].Radius;
		float angle = FRandSeeded() * 2.f * UKismetMathLibrary::GetPI();
		float s, c; SinCos(&s, &c, angle);
		float Dist = FRandSeeded() * RefRadius * 0.4f;
		FVector2D RandDir(c, s);
		RandDir = GetSafeNormal(RandDir);

		FVector NewCenter = RefCenter + FVector(RandDir.X, RandDir.Y, 0.f) * Dist;

		NewCenter = ClampToPlayableBounds(NewCenter, Radii[i], MapInfo->CachedPlayableBoundsForClients);

		StormCircles.push_back(FStormCircle{ NewCenter, Radii[i] });
	}

	for (int i = 7; i < 13; ++i)
	{
		FVector PrevCenter = StormCircles[i - 1].Center;
		float rPrev = StormCircles[i - 1].Radius;
		float rNew = Radii[i];

		float angle = FRandSeeded() * 2.f * UKismetMathLibrary::GetPI();
		float s, c; SinCos(&s, &c, angle);
		FVector2D RandDir(c, s);
		RandDir = GetSafeNormal(RandDir);

		float d = sqrt(rPrev * rPrev - rNew * rNew);;
		FVector NewCenter = PrevCenter + FVector(RandDir.X, RandDir.Y, 0.f) * d;

		NewCenter = ClampToPlayableBounds(NewCenter, rNew, MapInfo->CachedPlayableBoundsForClients);

		StormCircles.push_back(FStormCircle{ NewCenter, rNew });
	}
}

void FortGameStateComponent::SpawnInitialSafeZone(UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GameStateComponent, AFortGameModeAthena* GameMode, AFortGameStateAthena* GameState)
{
	GenerateStormCircles(GameState->MapInfo);

	AFortSafeZoneIndicator* SafeZoneIndicator = nullptr;

	if (!SafeZoneIndicator && GameStateComponent->SafeZoneIndicator)
		SafeZoneIndicator = GameStateComponent->SafeZoneIndicator;

	if (!SafeZoneIndicator)
		SafeZoneIndicator = Util->SpawnActor<AFortSafeZoneIndicator>({}, {}, GameStateComponent->SafeZoneIndicatorClass);

	FFortSafeZoneDefinition& SafeZoneDefinition = GameState->MapInfo->SafeZoneDefinition;
	UCurveTable* CurveTable = GameState->AthenaGameDataTable;
	auto& PhaseArray = SafeZoneIndicator->SafeZonePhases;

	if (PhaseArray.IsValid())
		PhaseArray.Free();

	const float TimeSeconds = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());

	const int TotalPhases = Util->ReadCurve(SafeZoneDefinition.Count, 0);

	for (int i = 0; i < TotalPhases; i++)
	{
		FFortSafeZonePhaseInfo SafeZonePhaseInfo{};

		SafeZonePhaseInfo.Radius = Util->ReadCurve(SafeZoneDefinition.Radius, i);
		SafeZonePhaseInfo.WaitTime = Util->ReadCurve(SafeZoneDefinition.WaitTime, i);
		SafeZonePhaseInfo.ShrinkTime = Util->ReadCurve(SafeZoneDefinition.ShrinkTime, i);
		SafeZonePhaseInfo.PlayerCap = Util->ReadCurve(SafeZoneDefinition.PlayerCapSolo, i);
		SafeZonePhaseInfo.MegaStormGridCellThickness = Util->ReadCurve(SafeZoneDefinition.MegaStormGridCellThickness, i);

		UDataTableFunctionLibrary::EvaluateCurveTableRow(CurveTable, UKismetStringLibrary::Conv_StringToName(L"Default.SafeZone.Damage"), i, nullptr, &SafeZonePhaseInfo.DamageInfo.Damage, FString());

		SafeZonePhaseInfo.TimeBetweenStormCapDamage = 1.f;
		SafeZonePhaseInfo.StormCapDamagePerTick = 1.f;
		SafeZonePhaseInfo.StormCampingIncrementTimeAfterDelay = 1.f;
		SafeZonePhaseInfo.StormCampingInitialDelayTime = 1.f;

		FFortSafeZoneDamageInfo& DamageInfo = SafeZonePhaseInfo.DamageInfo;
		DamageInfo.Damage = (i == 0) ? 0.01f : 1.f;
		DamageInfo.bPercentageBasedDamage = true;

		SafeZonePhaseInfo.Center = StormCircles[i].Center;

		PhaseArray.Add(SafeZonePhaseInfo);

		SafeZoneIndicator->PhaseCount++;
	}

	SafeZoneIndicator->OnRep_PhaseCount();

	SafeZoneIndicator->SafeZoneStartShrinkTime = TimeSeconds + PhaseArray[0].WaitTime;
	SafeZoneIndicator->SafeZoneFinishShrinkTime = SafeZoneIndicator->SafeZoneStartShrinkTime + PhaseArray[0].ShrinkTime;

	//Util->Logger(Info, "SpawnInitialSafeZone", std::format("SafeZoneStartShrinkTime={} SafeZoneFinishShrinkTime={}", SafeZoneIndicator->GetSafeZoneStartShrinkTime(), SafeZoneIndicator->GetSafeZoneFinishShrinkTime()).c_str());

	SafeZoneIndicator->CurrentPhase = 0;
	SafeZoneIndicator->OnRep_CurrentPhase();

	SafeZoneIndicator->Radius = PhaseArray[0].Radius;
	SafeZoneIndicator->NextRadius = PhaseArray[0].Radius;
	SafeZoneIndicator->NextCenter = (FVector_NetQuantize100&)PhaseArray[0].Center;

	*(uint8*)(__int64(SafeZoneIndicator->FutureReplicator) - 0x4) = 1;

	SafeZoneIndicator->ForceNetUpdate();

	if (!GameStateComponent->SafeZoneIndicator)
		GameStateComponent->SafeZoneIndicator = SafeZoneIndicator;

	GameStateComponent->OnRep_SafeZoneIndicator();
}

void FortGameStateComponent::StartNewSafeZonePhase(UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GameStateComponent)
{
	AFortSafeZoneIndicator* SafeZoneIndicator = GameStateComponent->SafeZoneIndicator;
	if (!SafeZoneIndicator)
		return;

	auto& PhaseArray = SafeZoneIndicator->SafeZonePhases;
	if (!PhaseArray.IsValid())
		return;

	int CurrentPhase = SafeZoneIndicator->CurrentPhase;
	int NextPhase = CurrentPhase + 1;

	if (NextPhase >= PhaseArray.Num())
		return;

	float Damage = 1.f;
	switch (CurrentPhase)
	{
	case 4:
		Damage = 2.f;
		break;
	case 5:
		Damage = 2.f;
		break;
	case 6:
		Damage = 5.f;
		break;
	case 7:
		Damage = 8.f;
		break;
	case 8:
		Damage = 10.f;
		break;
	case 9:
		Damage = 10.f;
		break;
	default:
		Damage = 1.f;
		break;
	}

	const float TimeSeconds = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
	const FFortSafeZonePhaseInfo& SafeZonePhaseInfo = PhaseArray[NextPhase];

	SafeZoneIndicator->CurrentPhase = NextPhase;
	SafeZoneIndicator->OnRep_CurrentPhase();

	SafeZoneIndicator->PreviousCenter = (FVector_NetQuantize100&)PhaseArray[CurrentPhase].Center;
	SafeZoneIndicator->PreviousRadius = PhaseArray[CurrentPhase].Radius;

	SafeZoneIndicator->NextRadius = SafeZonePhaseInfo.Radius;
	SafeZoneIndicator->NextCenter = (FVector_NetQuantize100&)SafeZonePhaseInfo.Center;
	SafeZoneIndicator->NextNextMegaStormGridCellThickness = SafeZonePhaseInfo.MegaStormGridCellThickness;

	SafeZoneIndicator->SafeZoneStartShrinkTime = TimeSeconds + SafeZonePhaseInfo.WaitTime;
	SafeZoneIndicator->SafeZoneFinishShrinkTime = SafeZoneIndicator->SafeZoneStartShrinkTime + SafeZonePhaseInfo.ShrinkTime;

	SafeZoneIndicator->Radius = PhaseArray[CurrentPhase].Radius;

	/*SafeZoneIndicator->CurrentDamageInfo = SafeZonePhaseInfo.DamageInfo;
	SafeZoneIndicator->OnRep_CurrentDamageInfo();*/

	SafeZoneIndicator->CurrentDamageInfo.Damage = Damage;
	SafeZoneIndicator->CurrentDamageInfo.bPercentageBasedDamage = false;
	SafeZoneIndicator->OnRep_CurrentDamageInfo();

	SafeZoneIndicator->ForceNetUpdate();
}
