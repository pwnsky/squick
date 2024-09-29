#pragma

#include "i_actor_module.h"
#include "i_component_module.h"
#include <map>
#include <core/guid.h>
#include <core/map_ex.h>
#include <core/platform.h>
#include <core/queue.h>
#include <string>

class Actor : public IActor {
  public:
    Actor(const Guid id, IActorModule *pModule);
    virtual ~Actor();
    const Guid ID();
    virtual bool Update();
    virtual bool AddComponent(std::shared_ptr<IComponent> component);
    virtual bool RemoveComponent(const std::string &componentName);
    virtual std::shared_ptr<IComponent> FindComponent(const std::string &componentName);
    virtual bool SendMsg(const ActorMessage &message);
    virtual bool SendMsg(const int eventID, const std::string &data, const std::string &arg);
    virtual bool BackMsgToMainThread(const ActorMessage &message);
    virtual bool AddMessageHandler(const int nSubMsgID, ACTOR_PROCESS_FUNCTOR_PTR xBeginFunctor);
    virtual void ToMemoryCounterString(std::string &info);

  protected:
    Guid id;
    IActorModule *m_pActorModule;
    Queue<ActorMessage> mMessageQueue;
    MapEx<std::string, IComponent> mComponent;
    MapEx<int, ACTOR_PROCESS_FUNCTOR> mxProcessFunctor;
};
