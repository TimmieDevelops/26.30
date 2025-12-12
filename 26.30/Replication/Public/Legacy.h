#pragma once
#include "../../pch.h"

struct FActorPriority
{
public:
	UActorChannel* Channel;
	FNetworkObjectInfo* ActorInfo;
public:
	FActorPriority();
	FActorPriority(UActorChannel* InChannel, FNetworkObjectInfo* InActorInfo);
};

class Legacy
{
public:
	UNetDriver* Driver;
	float DeltaSeconds;
public:
	Legacy() = default;
	Legacy(UNetDriver* InDriver, float DeltaTime);
public:
	void ServerReplicateActors();
	int32 PrepConnections();
	void BuildConsiderList(TArray<FNetworkObjectInfo*>& OutConsiderList, const float ServerTickTime);
	int32 PrioritizeActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionsViewers, const TArray<FNetworkObjectInfo*>& ConsiderList, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors);
	int32 ProcessPrioritizedActorsRange(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32& ActorsIndexRange, int32& OutUpdated);
	void MarkRelevantActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, int32 StartActorIndex, int32 EndActorIndex, FActorPriority** PriorityActors);
private:
	FNetViewer CreateNetViewer(UNetConnection* InConnection);
	bool IsLevelInitializedForActor(AActor* InActor, UNetConnection* InConnection);
	bool IsActorRelevantToConnection(AActor* Actor, const TArray<FNetViewer>& ConnectionViewers);
	bool IsActorDormant(FNetworkObjectInfo* ActorInfo, const TWeakObjectPtr<UNetConnection>& Connection);
	bool ShouldActorGoDormant(AActor* Actor, UActorChannel* Channel);
	AActor* GetNetOwner(AActor* Actor);
	UNetConnection* IsActorOwnedByAndRelevantToConnection(AActor* Actor, const TArray<FNetViewer>& ConnectionViewers, bool& bOutHadNullViewTarget);
};