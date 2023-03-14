#pragma once

#include <squick/core/consistent_hash.h>
//#include <squick/plugin/no_sql/export.h>
#include <squick/plugin/net/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/config/export.h>
#include <squick/core/base.h>

#include "../server/i_server_module.h"
#include "../client/i_db_module.h"
#include "../play/i_gameplay_manager_module.h"

#include "i_player_manager_module.h"
#include "i_room_module.h"

#include <unordered_map>

namespace game::player {
class PlayerManagerModule
    : public IPlayerManagerModule
{
public:
    PlayerManagerModule(IPluginManager* p)
    {
        pPluginManager = p;
    }

    virtual ~PlayerManagerModule() {};

    virtual bool Start();
    virtual bool Destory();
	virtual bool ReadyUpdate();
    virtual bool Update();
    virtual bool AfterStart();

	// 发送数据到客户端
	virtual void OnSendToClient(const uint16_t msgID, google::protobuf::Message& xMsg, const Guid& client_id);
	virtual Player* GetPlayer(const Guid& clientID) override;
	virtual int GetPlayerRoomID(const Guid& clientID) override;
	virtual void SetPlayerRoomID(const Guid& clientID, int roomID) override;
	virtual int GetPlayerGameplayID(const Guid& clientID) override;
	virtual void SetPlayerGameplayID(const Guid& clientID, int gameplayID) override;

protected:
	void OnReqPlayerEnter(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);
	void OnReqPlayerLeave(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);
	
	void OnAckPlayerDataLoad(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);
	int OnPlayerObjectEvent(const Guid & self, const std::string & className, const CLASS_OBJECT_EVENT classEvent, const DataList & var);

private:

	void LoadDataFromDb(const Guid& self);
	void SaveDataToDb(const Guid& self);

	int SaveDataOnTime(const Guid& self, const std::string& name, const float fIntervalTime, const int count);
	
private:

	// 待优化为 unordered_map , 查找时间复杂度为 O(1)
	std::map<Guid, SquickStruct::PlayerData> mxObjectDataCache;
	std::map<Guid, Player*> m_players; // 所有玩家
	std::map<Guid, Player*> m_offlineCachePlayers; // 离线缓存玩家
private:
	INetModule* m_pNetModule;
	IClassModule* m_pClassModule;
	IElementModule* m_pElementModule;
	IKernelModule* m_pKernelModule;
	ISceneModule* m_pSceneModule;
	IGameServerNet_ServerModule* m_pGameServerNet_ServerModule;
	IGameServerToDBModule* m_pGameToDBModule;
	INetClientModule* m_pNetClientModule;
	IScheduleModule* m_pScheduleModule;
	IDataTailModule* m_pDataTailModule;
	IEventModule* m_pEventModule;
	ILogModule* m_pLogModule;
	IRoomModule* m_pRoomModule;

	play::IGameplayManagerModule* m_pGameplayManagerModule;
};

}