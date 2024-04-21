#pragma once
#include <squick/core/guid.h>
#include <squick/core/i_module.h>
#include <squick/core/memory_counter.h>
#include <squick/core/platform.h>
#include <memory>

class ActorMessage;
class IComponent;

typedef std::function<void(ActorMessage &)> ACTOR_PROCESS_FUNCTOR;
typedef std::shared_ptr<ACTOR_PROCESS_FUNCTOR> ACTOR_PROCESS_FUNCTOR_PTR;

class ActorMessage {
  public:
    ActorMessage() {
        msg_id = 0;
        index = 0;
    }
    int msg_id;
    uint64_t index;
    Guid id;
    std::string data;
    std::string arg;

  protected:
  private:
};

class IActor // : MemoryCounter
{
  public:
    IActor()
    //: MemoryCounter(GET_CLASS_NAME(IActor), 1)
    {}

    virtual ~IActor() {}
    virtual const Guid ID() = 0;
    virtual bool Update() = 0;

    template <typename T> std::shared_ptr<T> AddComponent() {
        std::shared_ptr<IComponent> component = FindComponent(typeid(T).name());
        if (component) {
            return NULL;
        }

        {
            if (!TIsDerived<T, IComponent>::Result) {
                return NULL;
            }
            std::shared_ptr<T> component = std::shared_ptr<T>(new T());
            assert(NULL != component);
            AddComponent(component);
            return component;
        }
        return nullptr;
    }

    template <typename T> std::shared_ptr<T> FindComponent() {
        std::shared_ptr<IComponent> component = FindComponent(typeid(T).name());
        if (component) {
            std::shared_ptr<T> pT = std::dynamic_pointer_cast<T>(component);
            assert(NULL != pT);
            return pT;
        }
        return nullptr;
    }

    template <typename T> bool RemoveComponent() { return RemoveComponent(typeid(T).name()); }
    virtual bool SendMsg(const ActorMessage &message) = 0;
    virtual bool SendMsg(const int eventID, const std::string &data, const std::string &arg = "") = 0;
    virtual bool BackMsgToMainThread(const ActorMessage &message) = 0;
    virtual bool AddMessageHandler(const int nSubMsgID, ACTOR_PROCESS_FUNCTOR_PTR xBeginFunctor) = 0;

  protected:
    virtual bool AddComponent(std::shared_ptr<IComponent> component) = 0;
    virtual bool RemoveComponent(const std::string &componentName) = 0;
    virtual std::shared_ptr<IComponent> FindComponent(const std::string &componentName) = 0;
};

class IComponent // : MemoryCounter
{
  public:
    IComponent(const std::string &name) {
        mbEnable = true;
        mstrName = name;
    }

    virtual ~IComponent() {}
    virtual void SetActor(std::shared_ptr<IActor> self) { mSelf = self; }
    virtual std::shared_ptr<IActor> GetActor() { return mSelf; }
    virtual bool Awake() { return true; }
    virtual bool Start() { return true; }
    virtual bool AfterStart() { return true; }
    virtual bool CheckConfig() { return true; }
    virtual bool ReadyUpdate() { return true; }
    virtual bool Update() { return true; }
    virtual bool BeforeDestroy() { return true; }
    virtual bool Destroy() { return true; }
    virtual bool Finalize() { return true; }
    virtual bool SetEnable(const bool bEnable) {
        mbEnable = bEnable;
        return mbEnable;
    }

    virtual bool Enable() { return mbEnable; }
    virtual const std::string &GetComponentName() const { return mstrName; };
    virtual void ToMemoryCounterString(std::string &info) {
        info.append(mSelf->ID().ToString());
        info.append(":");
        info.append(mstrName);
    }

    template <typename BaseType> bool AddMsgHandler(const int nSubMessage, BaseType *pBase, int (BaseType::*handler)(ActorMessage &)) {
        ACTOR_PROCESS_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1);
        ACTOR_PROCESS_FUNCTOR_PTR functorPtr(new ACTOR_PROCESS_FUNCTOR(functor));
        return mSelf->AddMessageHandler(nSubMessage, functorPtr);
    }

  private:
    bool mbEnable;
    std::shared_ptr<IActor> mSelf;
    std::string mstrName;
};