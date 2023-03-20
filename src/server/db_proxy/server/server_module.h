#pragma once

#include <squick/core/map.h>
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/kernel/i_thread_pool_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_module.h>
#include <squick/struct/struct.h>

#include "../logic/i_account_redis_module.h"
#include "../logic/i_player_redis_module.h"
#include "i_server_module.h"
#include <squick/plugin/net/export.h>
// #include <server/game/logic/i_scene_process_module.h>
// #include <server/world/server/i_server_module.h>
// #include <server/world/client/i_master_module.h>

class DBNet_ServerModule : public IDBNet_ServerModule {
  public:
    DBNet_ServerModule(IPluginManager *p) { pPluginManager = p; }

    virtual bool Awake();
    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();

  protected:
    void OnSocketEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet *pNet);

    void OnClientDisconnect(const SQUICK_SOCKET nAddress);
    void OnClientConnected(const SQUICK_SOCKET nAddress);

    void OnRequireRoleListProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnCreateRoleGameProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnDeleteRoleGameProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnLoadRoleDataProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnSaveRoleDataProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

  protected:
    IPlayerRedisModule *m_pPlayerRedisModule;
    IAccountRedisModule *m_pAccountRedisModule;
    IElementModule *m_pElementModule;
    IClassModule *m_pClassModule;
    IKernelModule *m_pKernelModule;
    ILogModule *m_pLogModule;
    INetModule *m_pNetModule;
    INetClientModule *m_pNetClientModule;
    IThreadPoolModule *m_pThreadPoolModule;
};