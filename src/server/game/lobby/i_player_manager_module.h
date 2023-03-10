#pragma once

#include "player.h"

namespace game::player {

class IPlayerManagerModule : public IModule
{
public:
	virtual Player* GetPlayer(const Guid& clientID) = 0;
	virtual int GetPlayerRoomID(const Guid& clientID) = 0;
	virtual void SetPlayerRoomID(const Guid& clientID, int roomID) = 0;
	virtual int GetPlayerGameplayID(const Guid& clientID) = 0;
	virtual void SetPlayerGameplayID(const Guid& clientID, int gameplayID) = 0;
};

}