#pragma once

#include "squick/core/map.h"
#include <squick/struct/struct.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/kernel/i_thread_pool_module.h>
#include <squick/plugin/net/i_net_module.h>
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/log/i_log_module.h>


#include "../client/i_master_module.h"
//#include "world_logic.h"
#include "i_server_module.h"

class WorldNet_ServerModule
    : public IWorldNet_ServerModule
{
public:
    WorldNet_ServerModule(IPluginManager* p)
    {
        m_bIsUpdate = true;
        pPluginManager = p;
        mnLastCheckTime = pPluginManager->GetNowTime();
    }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();
	virtual void OnServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);

	virtual bool IsPrimaryWorldServer();
	virtual int GetWorldAreaID();

	virtual bool SendMsgToGame(const int gameID, const int msgID, const std::string& xData);
	virtual bool SendMsgToGame(const int gameID, const int msgID, const google::protobuf::Message& xData);

    virtual bool SendMsgToGamePlayer(const Guid nPlayer, const int msgID, const std::string& xData);
    virtual bool SendMsgToGamePlayer(const Guid nPlayer, const int msgID, const google::protobuf::Message& xData);
    virtual bool SendMsgToGamePlayer(const DataList& argObjectVar, const int msgID, google::protobuf::Message& xData);

    virtual SQUICK_SHARE_PTR<ServerData> GetSuitProxyToEnter();
	virtual SQUICK_SHARE_PTR<ServerData> GetSuitGameToEnter(const int arg);

    virtual int GetPlayerGameID(const Guid self);
    virtual const std::vector<Guid>& GetOnlinePlayers();

    virtual SQUICK_SHARE_PTR<IWorldNet_ServerModule::PlayerData> GetPlayerData(const Guid& id);

protected:
	virtual bool AddOnLineReceiveCallBack(std::shared_ptr<std::function<void(const Guid)>> cb);
	virtual bool AddOffLineReceiveCallBack(std::shared_ptr<std::function<void(const Guid)>> cb);
protected:

    void OnSocketEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet* pNet);

    void OnClientDisconnect(const SQUICK_SOCKET nAddress);
    void OnClientConnected(const SQUICK_SOCKET nAddress);


    void OnOnlineProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);
    void OnOfflineProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);

    void OnTransmitServerReport(const SQUICK_SOCKET nFd, const int msgId, const char *buffer, const uint32_t len);
    void ServerReport(int reportServerId, SquickStruct::ServerState serverStatus);

protected:

    void OnGameServerRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);
    void OnGameServerUnRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);
    void OnRefreshGameServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);

    void OnProxyServerRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);
    void OnProxyServerUnRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);
    void OnRefreshProxyServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);

	void OnDBServerRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);
	void OnDBServerUnRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);
	void OnRefreshDBServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);

    void OnGameplayManagerServerRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);
	void OnGameplayManagerServerUnRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);
	void OnRefreshGameplayManagerServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);

	void SynGameToProxy();
    void SynGameToProxy(const SQUICK_SOCKET nFD);

	void SynWorldToProxy();
	void SynWorldToProxy(const SQUICK_SOCKET nFD);

	void SynWorldToGame();
	void SynWorldToGame(const SQUICK_SOCKET nFD);

	void SynWorldToDB();
	void SynWorldToDB(const SQUICK_SOCKET nFD);

	void SynDBToGame();
	void SynDBToGame(const SQUICK_SOCKET nFD);


    // Gameplay Manager < -> World
    void SynGameToGameplayManager();
    void SynGameToGameplayManager(const SQUICK_SOCKET nFD);

    void SynWorldToGameplayManager();
    void SynWorldToGameplayManager(const SQUICK_SOCKET nFD);

    void LogGameServer();

private:
	int mAreaID = 0;

	std::vector<std::shared_ptr<std::function<void(const Guid)>>> mPlayerOnLineCallBackFunc;
	std::vector<std::shared_ptr<std::function<void(const Guid)>>> mPlayerOffLineCallBackFunc;

	MapEx<Guid, PlayerData> mPlayersData;
    INT64 mnLastCheckTime;

    //同一区服，所有服务器
	NFConsistentHashMapEx<int, ServerData> mWorldMap;
	NFConsistentHashMapEx<int, ServerData> mGameMap;
	NFConsistentHashMapEx<int, ServerData> mProxyMap;
	NFConsistentHashMapEx<int, ServerData> mDBMap;
    NFConsistentHashMapEx<int, ServerData> mGameplayManagerMap;

    IElementModule* m_pElementModule;
    IClassModule* m_pClassModule;
    IKernelModule* m_pKernelModule;
    ILogModule* m_pLogModule;
	INetModule* m_pNetModule;
	INetClientModule* m_pNetClientModule;
    IThreadPoolModule* m_pThreadPoolModule;
};
