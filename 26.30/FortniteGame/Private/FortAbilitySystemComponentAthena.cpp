#include "../Public/FortAbilitySystemComponentAthena.h"

void FortAbilitySystemComponentAthena::Init(UAbilitySystemComponent* AbilitySystemComponent)
{
	GiveAbilitySet(AbilitySystemComponent, UObject::FindObject<UFortAbilitySet>("FortAbilitySet GAS_AthenaPlayer.GAS_AthenaPlayer"));
	GiveAbilitySet(AbilitySystemComponent, UObject::FindObject<UFortAbilitySet>("FortAbilitySet AS_Ascender.AS_Ascender"));
	GiveAbilitySet(AbilitySystemComponent, UObject::FindObject<UFortAbilitySet>("FortAbilitySet AS_TacticalSprint.AS_TacticalSprint"));
}

void FortAbilitySystemComponentAthena::GiveAbilitySet(UAbilitySystemComponent* AbilitySystemComponent, UFortAbilitySet* AbilitySet)
{
	if (!AbilitySystemComponent || !AbilitySet)
		return;

	for (int i = 0; i < AbilitySet->GameplayAbilities.Num(); i++)
	{
		UClass* StaticClass = AbilitySet->GameplayAbilities[i];
		if (!StaticClass)
			continue;

		FGameplayAbilitySpec Spec;
		SpecConstructor(&Spec, StaticClass->DefaultObject->Cast<UGameplayAbility>(), 1, -1);
		AbilitySystemComponent->K2_GiveAbility(StaticClass, Spec.Level, Spec.InputID);
	}
}

void FortAbilitySystemComponentAthena::SpecConstructor(FGameplayAbilitySpec* Spec, UGameplayAbility* Ability, int a3, int a4, UObject* Object)
{
	void (*SpecConstructorOG)(FGameplayAbilitySpec * Spec, UGameplayAbility * Ability, int, int, UObject*) = decltype(SpecConstructorOG)(GetImageBase() + 0x6919894);
	return SpecConstructorOG(Spec, Ability, a3, a4, Object);
}
