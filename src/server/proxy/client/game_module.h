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
    GameModule(IPluginManager *p) { pPluginManager = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();

  protected:
    void OnSocketGSEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet *pNet);
    void Register(INet *pNet);
    void OnAckEnterGame(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void LogServerInfo(const std::string &strServerInfo);
    void Transport(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

  private:
    ILogModule *m_pLogModule;
    IKernelModule *m_pKernelModule;
    logic::ILogicModule *m_logic_;
    IElementModule *m_pElementModule;
    IClassModule *m_pClassModule;
    INetClientModule *m_pNetClientModule;
};

} // namespace proxy::client