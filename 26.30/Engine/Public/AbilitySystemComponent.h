#pragma once
#include "../../pch.h"

class AbilitySystemComponent
{
public:
	static AbilitySystemComponent* Get()
	{
		static AbilitySystemComponent* Instance = new AbilitySystemComponent();
		return Instance;
	}
public:
	static void Hook();
	static void InternalServerTryActivateAbility(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, FPredictionKey& PredictionKey, FGameplayEventData* TriggerEventData);
};