#pragma once
#include "../../pch.h"

class FortAbilitySystemComponentAthena
{
public:
	static FortAbilitySystemComponentAthena* Get()
	{
		static FortAbilitySystemComponentAthena* Instance = new FortAbilitySystemComponentAthena();
		return Instance;
	}
public:
	static void Init(UAbilitySystemComponent* AbilitySystemComponent);
	static void GiveAbilitySet(UAbilitySystemComponent* AbilitySystemComponent, UFortAbilitySet* AbilitySet);
	static void SpecConstructor(FGameplayAbilitySpec* Spec, UGameplayAbility* Ability, int a3, int a4, UObject* Object = nullptr);
	static FActiveGameplayEffectHandle ApplySetByCallerEffect(UAbilitySystemComponent* AbilitySystemComponent, UClass* StaticClass, FString TagName, float Level = 1.f, float Magnitude = 1.f);
};