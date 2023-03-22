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
        m_bIsUpdate = true;
        pPluginManager = p;
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
    void OnSocketWSEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet *pNet);

    void Register(INet *pNet);
    void ServerReport();

    void OnSelectServerResultProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

    void LogServerInfo(const std::string &strServerInfo);

    void OnOtherMessage(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

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

    ILogModule *m_pLogModule;
    logic::ILogicModule * logic_module_;
    IKernelModule *m_pKernelModule;
    server::IServerModule *server_module_;
    IElementModule *m_pElementModule;
    IClassModule *m_pClassModule;
    INetClientModule *m_pNetClientModule;
    ISecurityModule *m_pSecurityModule;
};

} // namespace proxy::client