#pragma once
#include <squick/core/base.h>
namespace proxy::logic {
class ILogicModule : public IModule {
  public:
    virtual int ForwardToClient(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) = 0;
    virtual int EnterSuccessEvent(const string xClientID, const string xPlayerID) = 0;
    virtual void OnClientConnected(const socket_t sock) = 0;

    virtual void OnAckConnectVerify(const int msg_id, const char *msg, const uint32_t len) = 0;

  protected:
  private:
};

} // namespace proxy::logic