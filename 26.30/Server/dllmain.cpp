// dllmain.cpp : Defines the entry point for the DLL application.
#include "../pch.h"

#include "../FortniteGame/Public/FortGameModeAthena.h"
#include "../FortniteGame/Public/FortInventoryServiceComponent.h"
#include "../FortniteGame/Public/FortPlayerStateZone.h"
#include "../FortniteGame/Public/FortControllerComponent_Aircraft.h"
#include "../FortniteGame/Public/FortPlayerController.h"
#include "../FortniteGame/Public/BuildingActor.h"
#include "../FortniteGame/Public/BuildingContainer.h"
#include "../FortniteGame/Public/FortPlayerControllerAthena.h"
#include "../FortniteGame/Public/FortPlayerPawn.h"
#include "../FortniteGame/Public/FortPlayerControllerZone.h"

#include "../Engine/Public/GameSession.h"
#include "../Engine/Public/Actor.h"
#include "../Engine/Public/World.h"
#include "../Engine/Public/NetDriver.h"
#include "../Engine/Public/UnrealEngine.h"
#include "../Engine/Public/GameModeBase.h"
#include "../Engine/Public/PlayerController.h"
#include "../Engine/Public/AbilitySystemComponent.h"
#include "../Engine/Public/ThreadHeartBeat.h"

inline static Utils* Util = Utils::Get();
inline static Globals* Global = Globals::Get();

static void WaitForLogin()
{
    Util->Logger(ELogType::Info, "WaitForLogin", "Waiting for login!");

    while (true)
    {
        UWorld* World = UWorld::GetWorld();
        if (World)
        {
            if (World->Name == UKismetStringLibrary::Conv_StringToName(L"Frontend"))
            {
                if (World->AuthorityGameMode->Cast<AGameMode>()->MatchState == UKismetStringLibrary::Conv_StringToName(L"InProgress"))
                    break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 1));
    Util->Logger(ELogType::Info, "WaitForLogin", "Logged In!");
}

inline static void* (*ProcessEventOG)(UObject* Object, UFunction* Function, void* Parameters);
static void* ProcessEvent(UObject* Object, UFunction* Function, void* Parameters)
{
    if (Function->GetName() == "OnDamageServer") Util->Logger(Info, "ProcessEvent", Function->GetFullName());
    //if (Function->GetName().contains("ServerHandlePickup")) Util->Logger(Info, "ProcessEvent", Function->GetFullName());
    return ProcessEventOG(Object, Function, Parameters);
}

static void HookAll()
{
    if (MH_Initialize() != MH_OK)
    {
        Util->Logger(ELogType::Warning, "HookAll", "Unable to initialize hooks");
        return;
    }

    PatchFunc::Get()->Hook();

    FortGameModeAthena::Get()->Hook();
    FortInventoryServiceComponent::Get()->Hook();
    FortPlayerStateZone::Get()->Hook();
    FortControllerComponent_Aircraft::Get()->Hook();
    FortPlayerController::Get()->Hook();
    BuildingActor::Get()->Hook();
    BuildingContainer::Get()->Hook();
    FortPlayerControllerAthena::Get()->Hook();
    FortPlayerPawn::Get()->Hook(); // stop being gay compile
    FortPlayerControllerZone::Get()->Hook();

    GameSession::Get()->Hook();
    Actor::Get()->Hook();
    World::Get()->Hook();
    NetDriver::Get()->Hook();
    UnrealEngine::Get()->Hook();
    GameModeBase::Get()->Hook();
    PlayerController::Get()->Hook();
    AbilitySystemComponent::Get()->Hook();
    ThreadHeartBeat::Get()->Hook();

    MH_CreateHook((LPVOID)(GetImageBase() + Offsets::ProcessEvent), ProcessEvent, (LPVOID*)(&ProcessEventOG));

    MH_EnableHook(MH_ALL_HOOKS);
}

DWORD MainThread(HMODULE Module)
{
    Util->InitConsole();
    Util->InitLogger();

    WaitForLogin();

    if (Global->bClient)
    {
        UEngine* Engine = UEngine::GetEngine();
        Engine->GameViewport->ViewportConsole = UGameplayStatics::SpawnObject(Engine->ConsoleClass, Engine->GameViewport)->Cast<UConsole>();
    }
    else
    {
        HookAll();
        Sleep(1000);
        //*(bool*)(GetImageBase() + 0xE83E1B2) = false; // GIsClient
        //*(bool*)(GetImageBase() + 0xE81611D) = true; // GIsServer
        //Util->Logger(ELogType::Info, "Main", std::format("{}", UFortRuntimeOptions::GetRuntimeOptions()->BRServerMaxTickRate).c_str());
        if (Global->bIris)
        {
            *(bool*)(GetImageBase() + 0xE981F30) = true; // net.Iris.UseIrisReplication
            //UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"net.Iris.UseIrisReplication 1", nullptr);
        }
        else
        {
            *(bool*)(GetImageBase() + 0xE71BFEC) = true; // GSetNetDormancyEnabled
        }

        UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogFortUIDirector", nullptr);
        UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"net.AllowEncryption 0", nullptr);
        UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogNet", nullptr);
        UGameplayStatics::OpenLevel(UWorld::GetWorld(), UKismetStringLibrary::Conv_StringToName(L"Asteria_Terrain"), false, FString());
        UWorld::GetWorld()->OwningGameInstance->LocalPlayers.Free();
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0);
        break;
    }

    return TRUE;
}
