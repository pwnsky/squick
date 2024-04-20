#pragma once
#include <squick/core/base.h>
namespace proxy::logic {
class ILogicModule : public IModule {
  public:
    virtual void OnClientDisconnected(const socket_t sock) = 0;

  protected:
  private:
};

} // namespace proxy::logic