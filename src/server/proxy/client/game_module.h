#pragma once

#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/struct/struct.h>
#include <string>

#include <squick/plugin/net/export.h>

#include <server/proxy/logic/i_logic_module.h>
#include "i_game_module.h"
namespace proxy::client {
class GameModule : public IGameModule {
  public:
    GameModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();

  protected:
    void OnSocketGSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);
    void Register(INet *pNet);
    void OnAckEnterGame(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void LogServerInfo(const std::string &strServerInfo);
    void Transport(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  private:
    ILogModule *m_log_;
    IKernelModule *m_kernel_;
    logic::ILogicModule *m_logic_;
    IElementModule *m_element_;
    IClassModule *m_class_;
    INetClientModule *m_net_client_;
};

} // namespace proxy::client