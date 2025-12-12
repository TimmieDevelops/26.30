#pragma once
#include "../../pch.h"

class BuildingActor
{
public:
	static BuildingActor* Get()
	{
		static BuildingActor* Instance = new BuildingActor();
		return Instance;
	}
public:
	static void Hook();
	static void OnDamageServer(ABuildingActor* Actor, float Damage, FGameplayTagContainer& DamageTags, FVector& Momentum, FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser, FGameplayEffectContextHandle& EffectContext);
};