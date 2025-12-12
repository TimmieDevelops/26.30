#include "../Public/FortGameModeAthena.h"
#include "../Public/FortGameStateComponent.h"

inline static FortGameStateComponent* GameStateComponent = FortGameStateComponent::Get();

inline static Utils* Util = Utils::Get();
inline static Globals* Global = Globals::Get();

void FortGameModeAthena::Hook()
{
    Util->HookVTable<AFortGameModeBR>(291, ReadyToStartMatch, nullptr);

    MH_CreateHook((LPVOID)(GetImageBase() + 0x79181F8), HandleMatchHasStarted, nullptr);
}

bool FortGameModeAthena::ReadyToStartMatch(AFortGameModeAthena* GameMode)
{
    static bool bInit = false;
    static bool bNetInit = false;

    static UEngine* Engine = UEngine::GetEngine();
    static UWorld* World = UWorld::GetWorld();

    static AFortGameStateAthena* GameState = GameMode->GameState->Cast<AFortGameStateAthena>();
    if (!GameMode || !GameState)
        return false;

    if (!GameState->MapInfo)
        return false;

    if (!bInit)
    {
        bInit = true;

        UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo");
        UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GamePhaseLogic = UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Get(World);

        auto& CurrentPlaylistInfo = GameState->CurrentPlaylistInfo;
        CurrentPlaylistInfo.BasePlaylist = Playlist;
        CurrentPlaylistInfo.OverridePlaylist = Playlist;
        CurrentPlaylistInfo.PlaylistReplicationKey++;
        CurrentPlaylistInfo.MarkArrayDirty();
        GameState->OnRep_CurrentPlaylistInfo();

        GameState->CurrentPlaylistId = Playlist->PlaylistId;
        GameState->OnRep_CurrentPlaylistId();

        GameMode->CurrentPlaylistId = Playlist->PlaylistId;
        GameMode->CurrentPlaylistName = Playlist->PlaylistName;
        GameMode->MinRespawnDelay = 5.0f;
        GameMode->WarmupRequiredPlayerCount = 1;
        GameMode->DefaultPawnClass = APlayerPawn_Athena_C::StaticClass();

        GamePhaseLogic->bGameModeWillSkipAircraft = Playlist->bSkipAircraft;
        GamePhaseLogic->AirCraftBehavior = Playlist->AirCraftBehavior;

        for (auto& Level : Playlist->AdditionalLevels)
        {
            bool Success = false;
            ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(World, Level, FVector(), FRotator(), &Success, FString(), nullptr, false);
            FAdditionalLevelStreamed LevelStreamed{};
            LevelStreamed.bIsServerOnly = false;
            LevelStreamed.LevelName = Level.ObjectID.AssetPath.AssetName;
            if (Success) GameState->AdditionalPlaylistLevelsStreamed.Add(LevelStreamed);
        }

        for (auto& Level : Playlist->AdditionalLevelsServerOnly)
        {
            bool Success = false;
            ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(World, Level, FVector(), FRotator(), &Success, FString(), nullptr, false);
            FAdditionalLevelStreamed LevelStreamed{};
            LevelStreamed.bIsServerOnly = false;
            LevelStreamed.LevelName = Level.ObjectID.AssetPath.AssetName;
            if (Success) GameState->AdditionalPlaylistLevelsStreamed.Add(LevelStreamed);
        }

        GameState->OnFinishedStreamingAdditionalPlaylistLevel();
        GameState->OnRep_AdditionalPlaylistLevelsStreamed();

        GameState->bIsUsingDownloadOnDemand = false;
        GameState->OnRep_IsUsingDownloadOnDemand();

        GameState->WorldLevel = Playlist->LootLevel;
    }

    if (!bNetInit)
    {
        bNetInit = true;

        if (World)
        {
            FName GameNetDriver = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");
            void* WorldContext = Engine->GetWorldContextFromWorld(World);
            UNetDriver* Driver = Engine->CreateNetDriver(WorldContext, GameNetDriver);

            if (Driver)
            {
                Driver->World = World;
                Driver->NetDriverName = GameNetDriver;
                World->NetDriver = Driver;

                FString Error;

                FURL URL;
                URL.Port = Global->ListenPort;

                if (Driver->InitListen(World, URL, false, Error))
                {
                    for (auto& LevelCollection : World->LevelCollections)
                        LevelCollection.NetDriver = Driver;

                    if (Global->bIris)
                        Driver->IsUsingIrisReplication() = true;

                    Driver->SetWorld(World);

                    GameMode->bWorldIsReady = true;
                    GameState->MapInfo->InitializeFlightPath(GameState, UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Get(World), false, 0.f, 0.f, 360.f);

                    GameStateComponent->SetGamePhase(World, EAthenaGamePhase::Setup);
                    GameStateComponent->SetGamePhaseStep(World, EAthenaGamePhaseStep::Setup);

                    Util->Logger(Info, "ReadyToStartMatch", std::format("{}", URL.Port).c_str());
                    SetConsoleTitle(std::format(L"Chapter 4 Season 4 | {}", URL.Port).c_str());
                }
            }
        }
    }

    if (GameMode->AlivePlayers.Num() > 0)
        return true;

    return false;
}

void FortGameModeAthena::HandleMatchHasStarted(AFortGameModeAthena* GameMode)
{
    UWorld* World = UWorld::GetWorld();
    AFortGameStateAthena* GameState = GameMode->GameState->Cast<AFortGameStateAthena>();
    auto& FlightInfo = GameState->MapInfo->FlightInfos[0];
    UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GamePhaseLogic = UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Get(World);

    AFortAthenaAircraft* Aircraft = AFortAthenaAircraft::SpawnAircraft(World, GameState->MapInfo->AircraftClass, FlightInfo);
    Aircraft->FlightInfo = FlightInfo;

    TWeakObjectPtr<AFortAthenaAircraft> WeakAircraft{};
    WeakAircraft.ObjectIndex = Aircraft->Index;

    GamePhaseLogic->Aircrafts_GameMode.Add(WeakAircraft);
    GamePhaseLogic->Aircrafts_GameState.Add(WeakAircraft);

    TArray<AFortAthenaAircraft*> Aircrafts;
    Aircrafts.Add(Aircraft);

    for (int i = 0; i < Aircrafts.Num(); i++)
        GamePhaseLogic->SetAircrafts(Aircrafts);

    GamePhaseLogic->OnRep_Aircrafts();

    GameStateComponent->SetAircraftTime(World, 60.f); //e 
    GameStateComponent->SetGamePhase(World, EAthenaGamePhase::Warmup);
    GameStateComponent->SetGamePhaseStep(World, EAthenaGamePhaseStep::Warmup);
}
