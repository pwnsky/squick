#pragma once

#include <squick/core/base.h>
namespace tutorial {
class IActorModule : public IModule {};

class ActorModule : public IActorModule {
public:
    ActorModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
    }
    
    virtual bool Start();
    virtual bool AfterStart();
    virtual bool Update();
};
}