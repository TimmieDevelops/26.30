#pragma once
#include "../../pch.h"

class Iris
{
public:
	UNetDriver* Driver;
	float DeltaSeconds;
public:
	Iris() = default;
	Iris(UNetDriver* InDriver, float DeltaTime);
public:
	void ServerReplicateActors();
};