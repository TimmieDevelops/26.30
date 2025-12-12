#include "../Public/GameSession.h"

void GameSession::Hook()
{
	MH_CreateHook((LPVOID)(GetImageBase() + 0x5C1EFEC), KickPlayer, nullptr);
}

bool GameSession::KickPlayer(AGameSession* GameSession, APlayerController* KickedPlayer, const FText& KickReason)
{
	return false;
}
