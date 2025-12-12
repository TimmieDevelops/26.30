#include "../Public/PlayerController.h"

inline static Utils* Util = Utils::Get();

void PlayerController::Hook()
{
	Util->HookVTable<AFortPlayerControllerAthena>(305, ServerAcknowledgePossession);
}

void PlayerController::SendClientAdjustment(APlayerController* PC)
{
	// Server sends updates.
	// Note: we do this for both the pawn and spectator in case an implementation has a networked spectator.
	ACharacter* RemotePawn = (ACharacter*)PC->Pawn;
	if (RemotePawn && (RemotePawn->RemoteRole == ENetRole::ROLE_AutonomousProxy))
	{
		//INetworkPredictionInterface* NetworkPredictionInterface = RemotePawn->GetMovementComponent()->Cast<INetworkPredictionInterface>();
		INetworkPredictionInterface* NetworkPredictionInterface = Utils::GetInterface<INetworkPredictionInterface>(RemotePawn->CharacterMovement);
		if (NetworkPredictionInterface)
		{
			NetworkPredictionInterface->SendClientAdjustment();
		}
	}
}

void PlayerController::ServerAcknowledgePossession(APlayerController* PC, APawn* P)
{
	PC->AcknowledgedPawn = P;
	UFortKismetLibrary::UpdatePlayerCustomCharacterPartsVisualization(PC->PlayerState->Cast<AFortPlayerState>());
}
