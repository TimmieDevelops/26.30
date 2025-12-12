#include "../Public/AbilitySystemComponent.h"

inline static Utils* Util = Utils::Get();

void AbilitySystemComponent::Hook()
{
	Util->HookVTable<UAbilitySystemComponent>(278, InternalServerTryActivateAbility);
	Util->HookVTable<UFortAbilitySystemComponent>(278, InternalServerTryActivateAbility);
	Util->HookVTable<UFortAbilitySystemComponentAthena>(278, InternalServerTryActivateAbility);
}

void AbilitySystemComponent::InternalServerTryActivateAbility(UAbilitySystemComponent* Ability, FGameplayAbilitySpecHandle Handle, bool InputPressed, FPredictionKey& PredictionKey, FGameplayEventData* TriggerEventData)
{
	FGameplayAbilitySpec* Spec = Ability->FindAbilitySpecFromHandle(Handle);
	if (!Spec)
	{
		Ability->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
		return;
	}

	UGameplayAbility* AbilityToActivate = Spec->Ability;
	if (!AbilityToActivate)
	{
		Ability->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
		return;
	}

	UGameplayAbility* InstancedAbility = nullptr;
	Spec->InputPressed = true;

	if (!Ability->InternalTryActivateAbility(Handle, PredictionKey, &InstancedAbility, nullptr, TriggerEventData))
	{
		Ability->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
		Spec->InputPressed = false;
		Ability->ActivatableAbilities.MarkItemDirty(Spec);
	}
}
