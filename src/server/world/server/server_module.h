#pragma once

#include "squick/core/map.h"
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/kernel/i_thread_pool_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_module.h>
#include <squick/struct/struct.h>

#include "../client/i_master_module.h"
// #include "world_logic.h"
#include "i_server_module.h"

class WorldNet_ServerModule : public IWorldNet_ServerModule {
  public:
    WorldNet_ServerModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
        mnLastCheckTime = pm_->GetNowTime();
    }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();
    virtual void OnServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    virtual bool IsPrimaryWorldServer();
    virtual int GetWorldAreaID();

    virtual bool SendMsgToGame(const int gameID, const int msg_id, const std::string &xData);
    virtual bool SendMsgToGame(const int gameID, const int msg_id, const google::protobuf::Message &xData);

    virtual bool SendMsgToGamePlayer(const Guid nPlayer, const int msg_id, const std::string &xData);
    virtual bool SendMsgToGamePlayer(const Guid nPlayer, const int msg_id, const google::protobuf::Message &xData);
    virtual bool SendMsgToGamePlayer(const DataList &argObjectVar, const int msg_id, google::protobuf::Message &xData);

    virtual std::shared_ptr<ServerData> GetSuitProxyToEnter();
    virtual std::shared_ptr<ServerData> GetSuitGameToEnter(const int arg);

    virtual int GetPlayerGameID(const Guid self);
    virtual const std::vector<Guid> &GetOnlinePlayers();

    virtual std::shared_ptr<IWorldNet_ServerModule::PlayerData> GetPlayerData(const Guid &id);

  protected:
    virtual bool AddOnLineReceiveCallBack(std::shared_ptr<std::function<void(const Guid)>> cb);
    virtual bool AddOffLineReceiveCallBack(std::shared_ptr<std::function<void(const Guid)>> cb);

  protected:
    void OnSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);

    void OnClientDisconnect(const socket_t sock);
    void OnClientConnected(const socket_t sock);

    void OnOnlineProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnOfflineProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    void OnTransmitServerReport(const socket_t nFd, const int msg_id, const char *buffer, const uint32_t len);
    void ServerReport(int reportServerId, rpc::ServerState serverStatus);

  protected:
    void OnGameServerRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnGameServerUnRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnRefreshGameServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    void OnProxyServerRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnProxyServerUnRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnRefreshProxyServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    void OnDBServerRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnDBServerUnRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnRefreshDBServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    void OnGameplayManagerServerRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnGameplayManagerServerUnRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnRefreshGameplayManagerServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    void SynGameToProxy();
    void SynGameToProxy(const socket_t nFD);

    void SynWorldToProxy();
    void SynWorldToProxy(const socket_t nFD);

    void SynWorldToGame();
    void SynWorldToGame(const socket_t nFD);

    void SynWorldToDB();
    void SynWorldToDB(const socket_t nFD);

    void SynDBToGame();
    void SynDBToGame(const socket_t nFD);

    // Gameplay Manager < -> World
    void SynGameToGameplayManager();
    void SynGameToGameplayManager(const socket_t nFD);

    void SynWorldToGameplayManager();
    void SynWorldToGameplayManager(const socket_t nFD);

    void LogGameServer();

  private:
    int mAreaID = 0;

    std::vector<std::shared_ptr<std::function<void(const Guid)>>> mPlayerOnLineCallBackFunc;
    std::vector<std::shared_ptr<std::function<void(const Guid)>>> mPlayerOffLineCallBackFunc;

    MapEx<Guid, PlayerData> mPlayersData;
    INT64 mnLastCheckTime;

    // 同一区服，所有服务器
    ConsistentHashMapEx<int, ServerData> mWorldMap;
    ConsistentHashMapEx<int, ServerData> mGameMap;
    ConsistentHashMapEx<int, ServerData> mProxyMap;
    ConsistentHashMapEx<int, ServerData> mDBMap;
    ConsistentHashMapEx<int, ServerData> mGameplayManagerMap;

    IElementModule *m_element_;
    IClassModule *m_class_;
    IKernelModule *m_kernel_;
    ILogModule *m_log_;
    INetModule *m_net_;
    INetClientModule *m_net_client_;
    IThreadPoolModule *m_thread_pool_;
};
