#include "../Public/Legacy.h"

#include "../../Engine/Public/PlayerController.h"
#include "../../Engine/Public/NetDriver.h"

inline static Globals* Global = Globals::Get();
inline static Utils* Util = Utils::Get();

inline static PlayerController* PC = PlayerController::Get();
inline static NetDriver* NetworkDriver = NetDriver::Get();

Legacy::Legacy(UNetDriver* InDriver, float DeltaTime) :
	Driver(InDriver),
	DeltaSeconds(DeltaTime)
{
}

void Legacy::ServerReplicateActors()
{
	if (!Driver || !Driver->World)
		return;

	if (Driver->ClientConnections.Num() == 0)
		return;

	Driver->GetReplicationFrame()++;

	const int32 NumClientsToTick = PrepConnections();
	if (!NumClientsToTick)
		return;

	int32 Updated = 0;

	AWorldSettings* WorldSettings = Driver->World->PersistentLevel->WorldSettings;

	float ServerTickTime = Global->MaxTickRate ? DeltaSeconds : 1.f / Global->MaxTickRate;

	TArray<FNetworkObjectInfo*> ConsiderList;
	ConsiderList.Reserve(Driver->GetNetworkObjectList().GetActiveObjects().Num());

	BuildConsiderList(ConsiderList, ServerTickTime);

	TArray<UNetConnection*> ConnectionsToClose;

	for (int32 i = 0; i < Driver->ClientConnections.Num(); i++)
	{
		UNetConnection* Connection = Driver->ClientConnections[i];
		if (!Connection)
			continue;

		if (i >= NumClientsToTick)
			continue;

		if (!Connection->ViewTarget)
			continue;

		TArray<FNetViewer>& ConnectionViewers = WorldSettings->ReplicationViewers;

		ConnectionViewers.Free();
		ConnectionViewers.Add(CreateNetViewer(Connection));

		for (UNetConnection* Child : Connection->Children)
		{
			if (Child->ViewTarget)
				ConnectionViewers.Add(CreateNetViewer(Child));
		}

		NetworkDriver->SendClientMoveAdjustments(Driver);

		FActorPriority* PriorityList = nullptr;
		FActorPriority** PriorityActors = nullptr;

		const int32 FinalSortedCount = PrioritizeActors(Connection, ConnectionViewers, ConsiderList, PriorityList, PriorityActors);
		const int32 LastProcessedActor = ProcessPrioritizedActorsRange(Connection, ConnectionViewers, PriorityActors, FinalSortedCount, Updated);

		MarkRelevantActors(Connection, ConnectionViewers, LastProcessedActor, FinalSortedCount, PriorityActors);

		ConnectionViewers.Free();

		Connection->GetLastProcessedFrame() = Driver->GetReplicationFrame();

		if (Connection->GetPendingCloseDueToReplicationFailure())
			ConnectionsToClose.Add(Connection);
	}

	for (UNetConnection* ConnectionToClose : ConnectionsToClose)
		ConnectionToClose->Close();
}

int32 Legacy::PrepConnections()
{
	int32 NumClientsToTick = Driver->ClientConnections.Num();

	bool bFoundReadyConnection = false;

	for (UNetConnection* Connection : Driver->ClientConnections)
	{
		if (!Connection)
			continue;

		AActor* OwningActor = Connection->OwningActor;
		if (OwningActor && (Connection->Driver->GetElapsedTime() - Connection->LastReceiveTime < 1.5))
		{
			if (Driver->World != OwningActor->GetWorld())
				continue;

			bFoundReadyConnection = true;

			AActor* DesiredViewTarget = OwningActor;
			if (Connection->PlayerController)
			{
				if (AActor* ViewTarget = Connection->PlayerController->GetViewTarget())
				{
					if (ViewTarget->GetWorld())
					{
						DesiredViewTarget = ViewTarget;
					}
				}
			}

			Connection->ViewTarget = DesiredViewTarget;

			for (UNetConnection* Child : Connection->Children)
			{
				APlayerController* ChildPC = Child->PlayerController;
				AActor* DesiredChildViewTarget = Child->OwningActor;

				if (ChildPC)
				{
					AActor* ChildViewTarget = ChildPC->GetViewTarget();

					if (ChildViewTarget && ChildViewTarget->GetWorld())
					{
						DesiredChildViewTarget = ChildViewTarget;
					}
				}

				Child->ViewTarget = DesiredChildViewTarget;
			}
		}
		else
		{
			Connection->ViewTarget = nullptr;

			for (UNetConnection* Child : Connection->Children)
				Child->ViewTarget = nullptr;
		}
	}

	return bFoundReadyConnection ? NumClientsToTick : 0;
}

void Legacy::BuildConsiderList(TArray<FNetworkObjectInfo*>& OutConsiderList, const float ServerTickTime)
{
	TArray<AActor*> ActorsToRemove;

	for (const TSharedPtr<FNetworkObjectInfo>& ObjectInfo : Driver->GetNetworkObjectList().GetActiveObjects())
	{
		FNetworkObjectInfo* ActorInfo = ObjectInfo.Get();

		if (!ActorInfo->bPendingNetUpdate && Driver->World->GetTimeSeconds() <= ActorInfo->NextUpdateTime)
			continue;

		AActor* Actor = ActorInfo->Actor;

		if (Actor->bActorIsBeingDestroyed)
		{
			ActorsToRemove.Add(Actor);
			continue;
		}

		if (Actor->GetRemoteRole() == ENetRole::ROLE_None)
		{
			ActorsToRemove.Add(Actor);
			continue;
		}

		if (Actor->NetDriverName != Driver->NetDriverName)
			continue;

		if (Driver->IsDormInitialStartupActor(Actor))
		{
			ActorsToRemove.Add(Actor);
			continue;
		}

		if (ActorInfo->LastNetReplicateTime == 0)
		{
			ActorInfo->LastNetReplicateTime = Driver->World->GetTimeSeconds();
			ActorInfo->OptimalNetUpdateDelta = 1.0f / Actor->NetUpdateFrequency;
		}

		const float ScaleDownStartTime = 2.0f;
		const float ScaleDownTimeRange = 5.0f;

		const float LastReplicateDelta = Driver->World->GetTimeSeconds() - ActorInfo->LastNetReplicateTime;

		if (LastReplicateDelta > ScaleDownStartTime)
		{
			if (Actor->MinNetUpdateFrequency == 0.0f)
				Actor->MinNetUpdateFrequency = 2.0f;

			const float MinOptimalDelta = 1.0f / Actor->NetUpdateFrequency;
			const float MaxOptimalDelta = UKismetMathLibrary::max_0(1.0f / Actor->MinNetUpdateFrequency, MinOptimalDelta);

			const float Alpha = UKismetMathLibrary::clamp((LastReplicateDelta - ScaleDownStartTime) / ScaleDownTimeRange, 0.0f, 1.0f);
			ActorInfo->OptimalNetUpdateDelta = UKismetMathLibrary::Lerp(MinOptimalDelta, MaxOptimalDelta, Alpha);
		}

		if (!ActorInfo->bPendingNetUpdate)
		{
			const float NextUpdateDelta = 1.0f / Actor->NetUpdateFrequency;

			ActorInfo->NextUpdateTime = Driver->World->GetTimeSeconds() + 0.01f * ServerTickTime + NextUpdateDelta;

			ActorInfo->LastNetUpdateTimestamp = Driver->GetElapsedTime();
		}

		ActorInfo->bPendingNetUpdate = false;

		OutConsiderList.Add(ActorInfo);

		Actor->CallPreReplication(Driver);

		//Util->Logger(ELogType::Info, "BuildConsiderList", Actor->GetFullName());
	}

	for (AActor* Actor : ActorsToRemove)
		Driver->World->RemoveNetworkActor(Actor);

	ActorsToRemove.Free();
}

int32 Legacy::PrioritizeActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionsViewers, const TArray<FNetworkObjectInfo*>& ConsiderList, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors)
{
	Driver->GetNetTag()++;

	for (int32 j = 0; j < Connection->SentTemporaries.Num(); j++)
		Connection->SentTemporaries[j]->NetTag = Driver->GetNetTag();

	int32 FinalSortedCount = 0;

	if (Driver->World != Connection->OwningActor->GetWorld())
	{
		Util->Logger(ELogType::Info, "PrioritizeActors", "or THIS");
		return FinalSortedCount;
	}

	TWeakObjectPtr<UNetConnection> WeakConnection;
	WeakConnection.ObjectIndex = Connection->Index;

	const int32 MaxSortedActors = ConsiderList.Num();
	if (MaxSortedActors > 0)
	{
		OutPriorityList = new FActorPriority[MaxSortedActors];
		OutPriorityActors = new FActorPriority * [MaxSortedActors];

		if (Driver->World != Connection->ViewTarget->GetWorld())
		{
			Util->Logger(ELogType::Info, "PrioritizeActors", "FUC");
			return FinalSortedCount;
		}

		for (FNetworkObjectInfo* ActorInfo : ConsiderList)
		{
			AActor* Actor = ActorInfo->Actor;

			UActorChannel* Channel = Connection->FindActorChannelRef(ActorInfo->WeakActor);

			if (!Channel)
			{
				if (!IsLevelInitializedForActor(Actor, Connection))
					continue;

				if (!IsActorRelevantToConnection(Actor, ConnectionsViewers))
					continue;
			}

			UNetConnection* PriorityConnection = Connection;

			if (Actor->bOnlyRelevantToOwner)
			{
				// i do this laterr...
				bool bHasNullViewTarget = false;

				PriorityConnection = IsActorOwnedByAndRelevantToConnection(Actor, ConnectionsViewers, bHasNullViewTarget);
				
				if (PriorityConnection == nullptr)
				{
					if (!bHasNullViewTarget && Channel && Driver->GetElapsedTime() - Channel->GetRelevantTime() >= Driver->RelevantTimeout)
						Channel->Close(EChannelCloseReason::Relevancy);
					continue;
				}
			}
			else if (*(bool*)(GetImageBase() + 0xE71BFEC))
			{
				if (IsActorDormant(ActorInfo, WeakConnection))
					continue;

				if (ShouldActorGoDormant(Actor, Channel))
					Channel->StartBecomingDormant();
			}

			if (Actor->NetTag != Driver->GetNetTag())
			{
				Actor->NetTag = Driver->GetNetTag(); // E

				OutPriorityList[FinalSortedCount] = FActorPriority(Channel, ActorInfo);
				OutPriorityActors[FinalSortedCount] = &OutPriorityList[FinalSortedCount];
				FinalSortedCount++;
			}
		}
	}

	return FinalSortedCount;
}

int32 Legacy::ProcessPrioritizedActorsRange(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32& ActorsIndexRange, int32& OutUpdated)
{
	if (!Connection->IsNetReady(false))
		return 0;

	for (int32 j = 0; j < ActorsIndexRange; j++)
	{
		FNetworkObjectInfo* ActorInfo = PriorityActors[j]->ActorInfo;
		if (!ActorInfo)
			continue;

		UActorChannel* Channel = PriorityActors[j]->Channel;
		if (!Channel || Channel->Actor)
		{
			AActor* Actor = ActorInfo->Actor;
			bool bIsRelevant = false;

			const bool bLevelInitializedForActor = IsLevelInitializedForActor(Actor, Connection);

			if (bLevelInitializedForActor)
			{
				if (!Actor->bTearOff && (!Channel || Driver->GetElapsedTime() - Channel->GetRelevantTime() > 1.0))
				{
					if (IsActorRelevantToConnection(Actor, ConnectionViewers))
					{
						bIsRelevant = true;
					}
				}
			}

			const bool bIsRecentlyRelevant = bIsRelevant || (Channel && Driver->GetElapsedTime() - Channel->GetRelevantTime() < Driver->RelevantTimeout) || (ActorInfo->ForceRelevantFrame >= Connection->GetLastProcessedFrame());

			if (bIsRecentlyRelevant)
			{
				if (!Channel)
				{
					if (bLevelInitializedForActor)
					{
						Channel = Connection->CreateChannelByName((FName*)(GetImageBase() + 0xEAEBC18), EChannelCreateFlags::OpenedLocally)->Cast<UActorChannel>();
						if (Channel)
							Channel->SetChannelActor(Actor, ESetChannelActorFlags::None);
					}
					else if (Actor->NetUpdateFrequency < 1.0f)
					{
						ActorInfo->NextUpdateTime = Driver->World->GetTimeSeconds() + 0.2f;
					}
				}

				if (Channel)
				{
					if (bIsRelevant)
					{
						Channel->GetRelevantTime() = Driver->GetElapsedTime() + 0.5;
					}

					if (Channel->Connection->IsNetReady(false))
					{
						if (Channel->ReplicateActor())
						{
							const float MinOptimalDelta = 1.0f / Actor->NetUpdateFrequency;
							const float MaxOptimalDelta = UKismetMathLibrary::max_0(1.0f / Actor->MinNetUpdateFrequency, MinOptimalDelta);
							const float DeltaReplications = (Driver->World->GetTimeSeconds() - ActorInfo->LastNetReplicateTime);

							ActorInfo->OptimalNetUpdateDelta = UKismetMathLibrary::clamp(DeltaReplications * 0.7f, MinOptimalDelta, MaxOptimalDelta);
							ActorInfo->LastNetReplicateTime = Driver->World->GetTimeSeconds();
						}

						OutUpdated++;
					}
					else
					{
						Actor->ForceNetUpdate();
					}

					if (!Connection->IsNetReady(false))
						return j;
				}
			}

			if ((!bIsRecentlyRelevant || Actor->bTearOff) && Channel)
			{
				if (!bLevelInitializedForActor || !Actor->IsNetStartupActor())
					Channel->Close(Actor->bTearOff ? EChannelCloseReason::TearOff : EChannelCloseReason::Relevancy);
			}
		}
	}

	return ActorsIndexRange;
}

void Legacy::MarkRelevantActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, int32 StartActorIndex, int32 EndActorIndex, FActorPriority** PriorityActors)
{
	for (int32 k = StartActorIndex; k < EndActorIndex; k++)
	{
		if (!PriorityActors[k]->ActorInfo)
			continue;

		AActor* Actor = PriorityActors[k]->ActorInfo->Actor;
		UActorChannel* Channel = PriorityActors[k]->Channel;

		if (Channel && Driver->GetElapsedTime() - Channel->GetRelevantTime() <= 1.0)
		{
			PriorityActors[k]->ActorInfo->bPendingNetUpdate = true;
		}
		else if (IsActorRelevantToConnection(Actor, ConnectionViewers))
		{
			PriorityActors[k]->ActorInfo->bPendingNetUpdate = true;
			if (Channel)
				Channel->GetRelevantTime() = Driver->GetElapsedTime() + 0.5;
		}

		if (PriorityActors[k]->ActorInfo->ForceRelevantFrame >= Connection->GetLastProcessedFrame())
			PriorityActors[k]->ActorInfo->ForceRelevantFrame = Driver->GetReplicationFrame() + 1;
	}
}

FNetViewer Legacy::CreateNetViewer(UNetConnection* InConnection)
{
	FNetViewer NetViewer;

	NetViewer.Connection = InConnection;
	NetViewer.InViewer = InConnection->PlayerController ? InConnection->PlayerController : InConnection->OwningActor;
	NetViewer.ViewTarget = InConnection->ViewTarget;

	if (!InConnection || !InConnection->OwningActor)
		return NetViewer;

	if (!(!InConnection->PlayerController || (InConnection->PlayerController != InConnection->OwningActor)))
		return NetViewer;

	APlayerController* ViewingController = InConnection->PlayerController;

	NetViewer.ViewLocation = NetViewer.ViewTarget->K2_GetActorLocation();
	if (ViewingController)
	{
		FRotator ViewRotation = ViewingController->GetControlRotation();
		static void (*GetPlayerViewPoint)(APlayerController*, FVector & out_Location, FRotator & out_Rotation) = decltype(GetPlayerViewPoint)(InSDKUtils::GetImageBase() + 0x2237AD8);
		GetPlayerViewPoint(ViewingController, NetViewer.ViewLocation, ViewRotation);
		NetViewer.ViewDir = UKismetMathLibrary::GetForwardVector(ViewRotation);
	}

	return NetViewer;
}

bool Legacy::IsLevelInitializedForActor(AActor* InActor, UNetConnection* InConnection)
{
	if (!InActor || !InConnection)
		return false;

	if (Driver->World != InActor->GetWorld())
		return false;

	const bool bCorrectWorld = Driver->WorldPackage && (InConnection->GetClientWorldPackageName() == Driver->WorldPackage->Name) && InConnection->ClientHasInitializedLevel(InActor);

	bool bIsConnectionPC = false;

	const UNetConnection* ParentConnection = InConnection->IsA(UChildConnection::StaticClass()) ? InConnection->Cast<UChildConnection>()->Parent : InConnection;

	if (InActor == ParentConnection->PlayerController)
	{
		bIsConnectionPC = true;
	}
	else
	{
		for (const UChildConnection* ChildConnection : ParentConnection->Children)
		{
			if (InActor == ChildConnection->PlayerController)
			{
				bIsConnectionPC = true;
				break;
			}
		}
	}

	return bCorrectWorld || bIsConnectionPC;
}

bool Legacy::IsActorRelevantToConnection(AActor* Actor, const TArray<FNetViewer>& ConnectionViewers)
{
	for (FNetViewer& Viewers : ConnectionViewers)
	{
		if (Actor->IsNetRelevantFor(Viewers.InViewer, Viewers.ViewTarget, Viewers.ViewLocation))
			return true;
	}

	return false;
}

bool Legacy::IsActorDormant(FNetworkObjectInfo* ActorInfo, const TWeakObjectPtr<UNetConnection>& Connection)
{
	return ActorInfo->DormantConnections.Contains(Connection);
}

bool Legacy::ShouldActorGoDormant(AActor* Actor, UActorChannel* Channel)
{
	if (Actor->NetDormancy <= ENetDormancy::DORM_Awake || !Channel || Channel->GetPendingDormancy() || Channel->GetDormant()) return false;
	if (Actor->NetDormancy == ENetDormancy::DORM_DormantPartial) return false;
	return true;
}

AActor* Legacy::GetNetOwner(AActor* Actor)
{
	if (Actor->IsA(APlayerController::StaticClass()))
		return Actor;

	if (AActor* Owner = Actor->GetOwner())
	{
		if (Owner->IsA(APlayerController::StaticClass()))
			return Owner;

		if (AActor* NetOwner = GetNetOwner(Owner))
			return NetOwner;
	}

	return nullptr;
}

UNetConnection* Legacy::IsActorOwnedByAndRelevantToConnection(AActor* Actor, const TArray<FNetViewer>& ConnectionViewers, bool& bOutHadNullViewTarget)
{
	AActor* ActorOwner = GetNetOwner(Actor);

	bOutHadNullViewTarget = false;

	for (FNetViewer& Viewers : ConnectionViewers)
	{
		UNetConnection* ViewerConnection = Viewers.Connection;

		if (ViewerConnection->ViewTarget == nullptr)
			bOutHadNullViewTarget = true;

		if (ActorOwner == ViewerConnection->PlayerController ||
			(ViewerConnection->PlayerController && ActorOwner == ViewerConnection->PlayerController->Pawn) ||
			(ViewerConnection->ViewTarget && ActorOwner == ViewerConnection->ViewTarget))
			return ViewerConnection;
	}

	return nullptr;
}

FActorPriority::FActorPriority() :
	ActorInfo(NULL),
	Channel(NULL)
{
}

FActorPriority::FActorPriority(UActorChannel* InChannel, FNetworkObjectInfo* InActorInfo) :
	ActorInfo(InActorInfo),
	Channel(InChannel)

{
}
