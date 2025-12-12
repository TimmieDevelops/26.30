#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <thread>

#include "SDK/SDK.hpp"
using namespace SDK;
using namespace InSDKUtils;

#include "includes/MinHook/MinHook.h"
#pragma comment(lib, "includes/MinHook/minhook.x64.lib")

enum ELogType
{
	Info,
	Warning,
	Error
};

enum class ENetMode : uint8
{
	Standalone,
	DedicatedServer,
	ListenServer,
	Client,

	MAX,
};