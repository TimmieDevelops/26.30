#include "../Public/FortAbilitySystemComponentAthena.h"

inline static Utils* Util = Utils::Get();

void FortAbilitySystemComponentAthena::Init(UAbilitySystemComponent* AbilitySystemComponent)
{
	GiveAbilitySet(AbilitySystemComponent, UObject::FindObject<UFortAbilitySet>("FortAbilitySet GAS_AthenaPlayer.GAS_AthenaPlayer"));
	GiveAbilitySet(AbilitySystemComponent, UObject::FindObject<UFortAbilitySet>("FortAbilitySet AS_Ascender.AS_Ascender"));
	GiveAbilitySet(AbilitySystemComponent, UObject::FindObject<UFortAbilitySet>("FortAbilitySet AS_TacticalSprint.AS_TacticalSprint"));
	ApplySetByCallerEffect(AbilitySystemComponent, UGE_OutsideSafeZoneDamage_C::StaticClass(), L"SetByCaller.StormCampingDamage");
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

FActiveGameplayEffectHandle FortAbilitySystemComponentAthena::ApplySetByCallerEffect(UAbilitySystemComponent* AbilitySystemComponent, UClass* StaticClass, const FString& TagName, float Level, float Magnitude)
{
	if (!AbilitySystemComponent)
		return FActiveGameplayEffectHandle();

	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(StaticClass, 1.f, Context);
	FActiveGameplayEffectHandle ActiveHandle = AbilitySystemComponent->BP_ApplyGameplayEffectSpecToSelf(SpecHandle);

	FGameplayTag StormTag{};
	StormTag.TagName = UKismetStringLibrary::Conv_StringToName(TagName);

	AbilitySystemComponent->UpdateActiveGameplayEffectSetByCallerMagnitude(ActiveHandle, StormTag, 1.f);
	return ActiveHandle;
}
