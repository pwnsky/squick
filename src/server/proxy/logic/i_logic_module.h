#pragma once
#include <squick/core/base.h>
namespace proxy::logic {
class ILogicModule : public IModule {
  public:
	  virtual int Transport(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) = 0;
	  virtual int EnterGameSuccessEvent(const Guid xClientID, const Guid xPlayerID) = 0;
	  virtual void OnClientConnected(const socket_t sock) = 0;
	  virtual void OnClientDisconnect(const socket_t sock) = 0;
  protected:
  private:
};

}