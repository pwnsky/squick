#pragma once
#include <squick/core/base.h>
namespace proxy::logic {
class ILogicModule : public IModule {
  public:
	  virtual int Transport(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len) = 0;
	  virtual int EnterGameSuccessEvent(const Guid xClientID, const Guid xPlayerID) = 0;
	  virtual void OnClientConnected(const SQUICK_SOCKET nAddress) = 0;
	  virtual void OnClientDisconnect(const SQUICK_SOCKET nAddress) = 0;
  protected:
  private:
};

}