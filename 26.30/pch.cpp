// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"

// When you are using pre-compiled headers, this source file is necessary for compilation to succeed.

void Utils::InitConsole()
{
    /* Code to open a console window */
    AllocConsole();
    FILE* Dummy;
    freopen_s(&Dummy, "CONOUT$", "w", stdout);
    freopen_s(&Dummy, "CONIN$", "r", stdin);
    SetConsoleTitle(L"Chapter 4 Season 4 | Starting...");
}

void Utils::InitLogger()
{
    if (std::filesystem::exists(LoggerName))
    {
        std::ofstream ofs(LoggerName, std::ofstream::trunc);
        ofs.close();
    }
    else
    {
        std::ofstream ofs(LoggerName);
        ofs.close();
    }
}

std::string Utils::GetLogType(ELogType LogType)
{
    std::string Information = "Info";

    switch (LogType)
    {
    case ELogType::Warning:
        Information = "Warning";
        break;
    case ELogType::Error:
        Information = "Error";
        break;
    }

    return Information;
}

void Utils::Logger(ELogType LogType, const std::string& Category, const std::string& Message)
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tmBuf;
    localtime_s(&tmBuf, &t);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::ostringstream timestamp;
    timestamp << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S") << "." << std::setw(3) << std::setfill('0') << ms.count();
    std::ofstream ofs(LoggerName, std::ios_base::app);
    std::string Information = GetLogType(LogType);
    ofs << "[" << timestamp.str() << "] [" << Information << "] [" << Category << "]: " << Message << std::endl;
    printf("[%s] [%s] [%s]: %s\n", timestamp.str().c_str(), Information.c_str(), Category.c_str(), Message.c_str());
}

void PatchFunc::Hook()
{
    for (uintptr_t Address : NullFunc)
        MH_CreateHook((LPVOID)(GetImageBase() + Address), IsNullptr, nullptr);

    for (uintptr_t Address : FalseNull)
        MH_CreateHook((LPVOID)(GetImageBase() + Address), IsFalse, nullptr);

    for (uintptr_t Address : TrueNull)
        MH_CreateHook((LPVOID)(GetImageBase() + Address), IsTrue, nullptr);

    MH_CreateHook((LPVOID)(GetImageBase() + 0x1060B88), DispatchRequest, (LPVOID*)(&DispatchRequestOG));
}

void* PatchFunc::IsNullptr()
{
    return nullptr;
}

bool PatchFunc::IsFalse()
{
    return false;
}

bool PatchFunc::IsTrue()
{
    return true;
}

void __fastcall PatchFunc::DispatchRequest(__int64 a1, __int64* a2, int a3)
{
    return DispatchRequestOG(a1, a2, 3);
}
