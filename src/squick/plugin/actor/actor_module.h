#pragma once

#include <map>
#include <queue>
#include <string>

#include <squick/core/queue.h>
#include <squick/plugin/utils/export.h>

#include "actor.h"
#include "i_actor_module.h"
#include "i_component_module.h"

class ActorModule : public IActorModule {
  public:
    ActorModule(IPluginManager *p);
    virtual ~ActorModule();
    virtual bool Start();
    virtual bool AfterStart();
    virtual bool BeforeDestory();
    virtual bool Destory();
    virtual bool Update();

    virtual std::shared_ptr<IActor> CreateActor();
    virtual std::shared_ptr<IActor> GetActor(const Guid nActorIndex);
    virtual bool ReleaseActor(const Guid nActorIndex);
    virtual bool SendMsgToActor(const Guid actorIndex, const Guid who, const int eventID, const std::string &data, const std::string &arg = "");
    virtual bool AddResult(const ActorMessage &message);

  protected:
    virtual bool SendMsgToActor(const Guid actorIndex, const ActorMessage &message);
    virtual bool AddEndFunc(const int subMessageID, ACTOR_PROCESS_FUNCTOR_PTR functorPtr_end);

    virtual bool UpdateEvent();
    virtual bool UpdateResultEvent();

  private:
    bool test = false;
    int64_t guid_index_ = 0;
    //IKernelModule *m_kernel_;
    IThreadPoolModule *m_thread_pool_;

    std::map<Guid, std::shared_ptr<IActor>> mxActorMap;
    Queue<ActorMessage> mxResultQueue;
    MapEx<int, ACTOR_PROCESS_FUNCTOR> mxEndFunctor;
};
