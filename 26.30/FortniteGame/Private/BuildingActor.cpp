#include "../Public/BuildingActor.h"
#include "../Public/FortInventoryServiceComponent.h"

inline static Utils* Util = Utils::Get();
inline static FortInventoryServiceComponent* InventoryService = FortInventoryServiceComponent::Get();

void BuildingActor::Hook()
{
	MH_CreateHook((LPVOID)(GetImageBase() + 0x7C470BC), OnDamageServer, nullptr);
}

void BuildingActor::OnDamageServer(ABuildingActor* Actor, float Damage, FGameplayTagContainer& DamageTags, FVector& Momentum, FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser, FGameplayEffectContextHandle& EffectContext)
{
	if (!Actor || !InstigatedBy)
		return;

	/*if (Actor->NetDormancy == ENetDormancy::DORM_DormantAll)
		Actor->SetNetDormancy(ENetDormancy::DORM_Awake);

	if (!Actor->bAlwaysRelevant)
		Actor->bAlwaysRelevant = true;*/

	//Util->Logger(Info, "OnDamageServer", std::format("NetDormancy={} bReplicates={} bAlwaysRelevant={}", static_cast<int>(Actor->NetDormancy), static_cast<bool>(Actor->bReplicates), static_cast<bool>(Actor->bAlwaysRelevant)).c_str());

	AFortPlayerControllerAthena* PC = InstigatedBy->Cast<AFortPlayerControllerAthena>();
	if (!PC || !PC->GetPlayerPawn())
		return;

	if (Actor->bPlayerPlaced || Actor->bDestroyed)
		return;

	AFortWeapon* Weapon = PC->GetPlayerPawn()->CurrentWeapon;
	if (!Weapon || !Weapon->WeaponData->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
		return;

	bool bWeakSpotHit = Damage == 100.f;

	ABuildingSMActor* BuildingActor = Actor->Cast<ABuildingSMActor>();
	if (!BuildingActor)
		return;

	UFortItemDefinition* ItemDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingActor->ResourceType);
	if (!ItemDefinition)
		return;

	int Amount = bWeakSpotHit ? UKismetMathLibrary::RandomIntegerInRange(10, 15) : UKismetMathLibrary::RandomIntegerInRange(5, 8);

	PC->ClientReportDamagedResourceBuilding(BuildingActor, BuildingActor->ResourceType, Amount, Actor->bDestroyed, bWeakSpotHit);
	InventoryService->AddItem(PC, ItemDefinition, Amount);
}
