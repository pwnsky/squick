#pragma once

#include <squick/struct/struct.h>

#include <squick/plugin/kernel/export.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <squick/plugin/security/export.h>

#include "../logic/i_logic_module.h"
#include "../server/i_server_module.h"
#include "i_game_module.h"
#include "i_world_module.h"

namespace proxy::client {
class WorldModule : public IWorldModule {
  public:
    WorldModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
        mLastReportTime = 0;
    }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();

    virtual void LogReceive(const char *str) {}
    virtual void LogSend(const char *str) {}

    virtual INetClientModule *GetClusterModule();
    virtual bool VerifyConnectData(const std::string &account, const std::string &strKey);

  protected:
    void OnSocketWSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);

    void Register(INet *pNet);
    void ServerReport();

    void OnSelectServerResultProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    void LogServerInfo(const std::string &strServerInfo);

    void OnOtherMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  private:
    struct ClientConnectData {
        ClientConnectData() {
            account = "";
            strConnectKey = "";
        }

        std::string account;
        std::string strConnectKey;
    };

    MapEx<std::string, ClientConnectData> mWantToConnectMap;

  private:
    INT64 mLastReportTime;

    ILogModule *m_log_;
    logic::ILogicModule * m_logic_;
    IKernelModule *m_kernel_;
    server::IServerModule *server_module_;
    IClassModule* m_class_;
    IElementModule *m_element_;
    
    INetClientModule *m_net_client_;
    ISecurityModule *m_security_;
};

} // namespace proxy::client