// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"
#include "includes/memcury.h"

class Globals
{
public:
	static Globals* Get()
	{
		static Globals* Instance = new Globals();
		return Instance;
	}
public:
	bool bClient = false;
	int32 ListenPort = 7777;
	ENetMode NetMode = ENetMode::DedicatedServer;
	bool bIris = false;
	float MaxTickRate = 30.f;
};

class Utils
{
public:
	static Utils* Get()
	{
		static Utils* Instance = new Utils();
		return Instance;
	}
public:
	inline static std::string LoggerName = "26.30.txt";
public:
	static void InitConsole();
	static void InitLogger();
	static std::string GetLogType(ELogType LogType);
	static void Logger(ELogType LogType, const std::string& Category, const std::string& Message);
public:
	template<typename T>
	static void HookVTable(int Index, void* Detour, void** Original = nullptr)
	{
		void** VTable = T::GetDefaultObj()->VTable;
		if (Original) *Original = VTable[Index];
		DWORD Protect;
		VirtualProtect(&VTable[Index], sizeof(void*), PAGE_EXECUTE_READWRITE, &Protect);
		VTable[Index] = Detour;
		VirtualProtect(&VTable[Index], sizeof(void*), Protect, &Protect);
	}

	template <typename _It>
	static inline _It* GetInterface(UObject* Object)
	{
		return ((_It * (*)(UObject*, UClass*)) (GetImageBase() + 0xD40B40))(Object, _It::StaticClass());
	}

	static FQuat RotatorToQuat(FRotator Rotator)
	{
		FQuat Quat{};

		auto DEG_TO_RAD = 3.14159 / 180;
		auto DIVIDE_BY_2 = DEG_TO_RAD / 2;

		auto SP = sin(Rotator.Pitch * DIVIDE_BY_2);
		auto CP = cos(Rotator.Pitch * DIVIDE_BY_2);

		auto SY = sin(Rotator.Yaw * DIVIDE_BY_2);
		auto CY = cos(Rotator.Yaw * DIVIDE_BY_2);

		auto SR = sin(Rotator.Roll * DIVIDE_BY_2);
		auto CR = cos(Rotator.Roll * DIVIDE_BY_2);

		Quat.X = CR * SP * SY - SR * CP * CY;
		Quat.Y = -CR * SP * CY - SR * CP * SY;
		Quat.Z = CR * CP * SY - SR * SP * CY;
		Quat.W = CR * CP * CY + SR * SP * SY;

		return Quat;
	}

	template<typename T = AActor>
	static T* SpawnActor(FVector Location = { 0,0,0 }, FRotator Rotation = { 0,0,0 }, UClass* StaticClass = T::StaticClass(), AActor* ActorOwner = nullptr)
	{
		FTransform Transform;
		Transform.Rotation = RotatorToQuat(Rotation);
		Transform.Scale3D = { 1,1,1 };
		Transform.Translation = Location;
		AActor* Actor = UGameplayStatics::BeginDeferredActorSpawnFromClass(UWorld::GetWorld(), StaticClass, Transform, ESpawnActorCollisionHandlingMethod::AlwaysSpawn, ActorOwner, ESpawnActorScaleMethod::SelectDefaultAtRuntime);
		if (Actor) return static_cast<T*>(UGameplayStatics::FinishSpawningActor(Actor, Transform, ESpawnActorScaleMethod::MultiplyWithRoot));
		return nullptr;
	}

	template<typename T>
	static void CreateFuncHook(const std::string& ClassName, const std::string& FuncName, void* Detour, void** Original = nullptr)
	{
		UFunction* Function = T::GetDefaultObj()->Class->GetFunction(ClassName, FuncName);
		if (!Function || !Function->ExecFunction) return;
		if (Original) *Original = reinterpret_cast<void*>(Function->ExecFunction); // i think?
		MH_CreateHook((LPVOID)(Function->ExecFunction), Detour, (LPVOID*)(Original));
	}

	// plooshi code
	template <typename _Is>
	static __forceinline void Patch(uintptr_t ptr, _Is byte)
	{
		DWORD og;
		VirtualProtect(LPVOID(ptr), sizeof(_Is), PAGE_EXECUTE_READWRITE, &og);
		*(_Is*)ptr = byte;
		VirtualProtect(LPVOID(ptr), sizeof(_Is), og, &og);
	}

	// plooshi code
	void PatchAllNetModes(uintptr_t AttemptDeriveFromURL)
	{
		Memcury::PE::Address add{ nullptr };

		const auto sizeOfImage = Memcury::PE::GetNTHeaders()->OptionalHeader.SizeOfImage;
		const auto scanBytes = reinterpret_cast<std::uint8_t*>(Memcury::PE::GetModuleBase());

		for (auto i = 0ul; i < sizeOfImage - 5; ++i)
		{
			if (scanBytes[i] == 0xE8 || scanBytes[i] == 0xE9)
			{
				if (Memcury::PE::Address(&scanBytes[i]).RelativeOffset(1).GetAs<void*>() == (void*)AttemptDeriveFromURL)
				{
					add = Memcury::PE::Address(&scanBytes[i]);

					// scan for the read of World->NetDriver

					for (auto j = 0; j > -0x100000; j--) // so we find everything. no func is actually 1mb
					{
						if ((scanBytes[i + j] & 0xF8) == 0x48 && ((scanBytes[i + j + 1] & 0xFC) == 0x80 || (scanBytes[i + j + 1] & 0xF8) == 0x38) && (scanBytes[i + j + 2] & 0xF0) != 0xC0 && (scanBytes[i + j + 2] & 0xF0) != 0xE0 && scanBytes[i + j + 2] != 0x65 && scanBytes[i + j + 2] != 0xBB && scanBytes[i + j + 3] == 0x38 && ((scanBytes[i + j + 1] & 0xFC) != 0x80 || scanBytes[i + j + 4] == 0x0))
						{
							// now, scan for if (NetDriver) return NM_Client;

							bool found = false;
							for (auto k = 4; k < 0x104; k++)
							{
								if (scanBytes[i + j + k] == 0x75)
								{
									auto Scuffness = __int64(&scanBytes[i + j + k + 5]);

									if (*(uint32_t*)Scuffness != 0xF0 && (scanBytes[i + j + k + 4] != 0xC || scanBytes[i + j + k + 5] != 0xB) && scanBytes[i + j + k + 4] != 0x09)
										continue;

									Patch<uint16_t>(__int64(&scanBytes[i + j + k]), 0x9090);
									if ((scanBytes[i + j + 1] & 0xF8) == 0x38)
										Patch<uint32_t>(__int64(&scanBytes[i + j]), 0x90909090);
									else if ((scanBytes[i + j + 1] & 0xFC) == 0x80)
									{
										DWORD og;
										VirtualProtect(&scanBytes[i + j], 5, PAGE_EXECUTE_READWRITE, &og);
										*(uint32*)(&scanBytes[i + j]) = 0x90909090;
										*(uint8*)(&scanBytes[i + j + 4]) = 0x90;
										VirtualProtect(&scanBytes[i + j], 5, og, &og);
									}
									FlushInstructionCache(GetCurrentProcess(), &scanBytes[i + j], 5);
									FlushInstructionCache(GetCurrentProcess(), &scanBytes[i + j + k], 2);
									found = true;
									break;
								}
								else if (scanBytes[i + j + k] == 0x74)
								{
									auto Scuffness = __int64(&scanBytes[i + j + k]);
									Scuffness = (Scuffness + 2) + *(int8_t*)(Scuffness + 1);

									if (*(uint32_t*)(Scuffness + 3) != 0xF0 && (*(uint8_t*)(Scuffness + 2) != 0xC || *(uint8_t*)(Scuffness + 3) != 0xB) && *(uint8_t*)(Scuffness + 2) != 0x09)
										continue;

									Patch<uint8_t>(__int64(&scanBytes[i + j + k]), 0xeb);
									if ((scanBytes[i + j + 1] & 0xF8) == 0x38)
										Patch<uint32_t>(__int64(&scanBytes[i + j]), 0x90909090);
									else if ((scanBytes[i + j + 1] & 0xFC) == 0x80)
									{
										DWORD og;
										VirtualProtect(&scanBytes[i + j], 5, PAGE_EXECUTE_READWRITE, &og);
										*(uint32*)(&scanBytes[i + j]) = 0x90909090;
										*(uint8*)(&scanBytes[i + j + 4]) = 0x90;
										VirtualProtect(&scanBytes[i + j], 5, og, &og);
									}
									FlushInstructionCache(GetCurrentProcess(), &scanBytes[i + j], 5);
									FlushInstructionCache(GetCurrentProcess(), &scanBytes[i + j + k], 1);
									found = true;
									break;
								}
								else if (scanBytes[i + j + k] == 0x0F && scanBytes[i + j + k + 1] == 0x85)
								{
									auto Scuffness = __int64(&scanBytes[i + j + k + 9]);

									if (*(uint32_t*)Scuffness != 0xF0 && (scanBytes[i + j + k + 8] != 0xC || scanBytes[i + j + k + 9] != 0xB) && scanBytes[i + j + k + 8] != 0x09)
										continue;

									DWORD og;
									VirtualProtect(&scanBytes[i + j + k], 6, PAGE_EXECUTE_READWRITE, &og);
									*(uint32*)(&scanBytes[i + j + k]) = 0x90909090;
									*(uint16*)(&scanBytes[i + j + k + 4]) = 0x9090;
									VirtualProtect(&scanBytes[i + j + k], 6, og, &og);
									if ((scanBytes[i + j + 1] & 0xF8) == 0x38)
										Patch<uint32_t>(__int64(&scanBytes[i + j]), 0x90909090);
									else if ((scanBytes[i + j + 1] & 0xFC) == 0x80)
									{
										DWORD og;
										VirtualProtect(&scanBytes[i + j], 5, PAGE_EXECUTE_READWRITE, &og);
										*(uint32*)(&scanBytes[i + j]) = 0x90909090;
										*(uint8*)(&scanBytes[i + j + 4]) = 0x90;
										VirtualProtect(&scanBytes[i + j], 5, og, &og);
									}
									FlushInstructionCache(GetCurrentProcess(), &scanBytes[i + j], 5);
									FlushInstructionCache(GetCurrentProcess(), &scanBytes[i + j + k], 6);
									found = true;
									break;
								}
								else if (scanBytes[i + j + k] == 0x0F && scanBytes[i + j + k + 1] == 0x84)
								{
									auto Scuffness = __int64(&scanBytes[i + j + k]);
									Scuffness = (Scuffness + 6) + *(int32_t*)(Scuffness + 2);

									if (*(uint32_t*)(Scuffness + 3) != 0xF0 && (*(uint8_t*)(Scuffness + 2) != 0xC || *(uint8_t*)(Scuffness + 3) != 0xB) && *(uint8_t*)(Scuffness + 2) != 0x09)
										continue;

									Patch<uint16_t>(__int64(&scanBytes[i + j + k]), 0xe990);
									if ((scanBytes[i + j + 1] & 0xF8) == 0x38)
										Patch<uint32_t>(__int64(&scanBytes[i + j]), 0x90909090);
									else if ((scanBytes[i + j + 1] & 0xFC) == 0x80)
									{
										DWORD og;
										VirtualProtect(&scanBytes[i + j], 5, PAGE_EXECUTE_READWRITE, &og);
										*(uint32*)(&scanBytes[i + j]) = 0x90909090;
										*(uint8*)(&scanBytes[i + j + 4]) = 0x90;
										VirtualProtect(&scanBytes[i + j], 5, og, &og);
									}
									FlushInstructionCache(GetCurrentProcess(), &scanBytes[i + j], 5);
									FlushInstructionCache(GetCurrentProcess(), &scanBytes[i + j + k], 2);
									found = true;
									break;
								}
							}
							if (found)
								break;
						}
					}
				}
			}
		}
	}

	template<typename T>
	static T* StaticLoadObject(const std::string& Name, UObject* Outer = nullptr, UClass* InClass = T::StaticClass())
	{
		static UObject* (*StaticLoadObjectOG)(UClass * Class, UObject * InOuter, const TCHAR * Name, const TCHAR * Filename, uint32_t LoadFlags, UPackageMap * Sandbox, bool bAllowObjectReconciliation, void* InSerializeContext) = decltype(StaticLoadObjectOG)(GetImageBase() + 0x166E984);
		return (T*)StaticLoadObjectOG(InClass, Outer, std::wstring(Name.begin(), Name.end()).c_str(), nullptr, 0, nullptr, false, nullptr);
	}

	static float ReadCurve(const FScalableFloat& ScalableFloat, float X = 0.f)
	{
		if (!ScalableFloat.Curve.CurveTable)
			return ScalableFloat.Value;

		float Out;
		UDataTableFunctionLibrary::EvaluateCurveTableRow(ScalableFloat.Curve.CurveTable, ScalableFloat.Curve.RowName, X, nullptr, &Out, FString());
		return Out;
	}
};

class PatchFunc
{
public:
	static PatchFunc* Get()
	{
		static PatchFunc* Instance = new PatchFunc();
		return Instance;
	}
public:
	inline static std::vector<uintptr_t> NullFunc = {
		0x4AC53E4, // "Engine exit requested"
		0x31E946C, // "FPlatformMisc::RequestExitWithStatus"
		0x4A6CE28, // "FPlatformMisc::RequestExit"
		0x33B7968, // "Changing GameSessionId from '' to '<NoGameSession>'"
	};

	inline static std::vector<uintptr_t> FalseNull = {
		0x3819AB4,
	};

	inline static std::vector<uintptr_t> TrueNull = {
		//0x5DAD70C, // "LogPlayerController: Warning: ServerUpdateLevelVisibility() Added '/ChronoEnviroItems/CB/Athena/Asteria/Maps/Asteria_Terrain/Generated/3D01O1X2WAXJAX92ZQB2OH2DJ', but level is not visible on server."
	};

public:
	static void Hook();
	static void* IsNullptr();
	static bool IsFalse();
	static bool IsTrue();
	static void __fastcall DispatchRequest(__int64 a1, __int64* a2, int a3);
private:
	inline static void (*DispatchRequestOG)(__int64 a1, __int64* a2, int a3);
};

struct FFrame
{
public:
	uint8*& GetCode()
	{
		return *(uint8**)(__int64(this) + 0x20);
	}

	UObject*& GetObjectContext()
	{
		return *(UObject**)(__int64(this) + 0x18);
	}

	FField*& GetPropertyinForCompiledIn()
	{
		return *(FField**)(__int64(this) + 0x88);
	}

public:
	void Step(UObject* Context, const void* Result)
	{
		static void (*StepOG)(FFrame*, UObject * Context, const void* Result) = decltype(StepOG)(GetImageBase() + 0xFB2034);
		return StepOG(this, Context, Result);
	}

	void StepExplicitProperty(const void* Result, FProperty* Property)
	{
		static void (*StepExplicitPropertyOG)(FFrame* Frame, const void* Result, FProperty* Property) = decltype(StepExplicitPropertyOG)(GetImageBase() + 0x129AAC0);
		return StepExplicitPropertyOG(this, Result, Property);
	}

	void StepCompiledIn(void* Result)
	{
		if (this->GetCode())
		{
			this->Step(this->GetObjectContext(), Result);
		}
		else
		{
			FProperty* Property = (FProperty*)this->GetPropertyinForCompiledIn();
			this->GetPropertyinForCompiledIn() = Property->Next;
			this->StepExplicitProperty(Result, Property);
		}
	}

	void IncrementCode()
	{
		this->GetCode() += this->GetCode() != 0;
	}
};

#endif //PCH_H
