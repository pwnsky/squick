

#include "actor.h"
#include <squick/core/i_plugin_manager.h>

Actor::Actor(const Guid id, IActorModule *pModule) {
    this->id = id;
    m_pActorModule = pModule;
}

Actor::~Actor() {}

const Guid Actor::ID() { return this->id; }

bool Actor::Update() {
    // bulk
    ActorMessage messageObject;
    while (mMessageQueue.TryPop(messageObject)) {
        // must make sure that only one thread running this function at the same time
        // mxProcessFunctor is not thread-safe
        ACTOR_PROCESS_FUNCTOR_PTR xBeginFunctor = mxProcessFunctor.GetElement(messageObject.msg_id);

        if (xBeginFunctor != nullptr) {
            // std::cout << ID().ToString() << " received message " << messageObject.msg_id << " and msg index is " << messageObject.index << " totaly msg
            // count: " << mMessageQueue.size_approx() << std::endl;

            xBeginFunctor->operator()(messageObject);

            // return the result to the main thread
            m_pActorModule->AddResult(messageObject);
        }
    }

    return true;
}

bool Actor::AddComponent(std::shared_ptr<IComponent> component) {
    // if you want to add more components for the actor, please don't clear the component
    // mComponent.ClearAll();
    if (!mComponent.ExistElement(component->GetComponentName())) {
        mComponent.AddElement(component->GetComponentName(), component);
        component->SetActor(std::shared_ptr<IActor>(this));

        component->Awake();
        component->Start();
        component->AfterStart();
        component->ReadyUpdate();

        return true;
    }

    return false;
}

bool Actor::RemoveComponent(const std::string &componentName) { return false; }

std::shared_ptr<IComponent> Actor::FindComponent(const std::string &componentName) { return mComponent.GetElement(componentName); }

bool Actor::AddMessageHandler(const int nSubMsgID, ACTOR_PROCESS_FUNCTOR_PTR xBeginFunctor) { return mxProcessFunctor.AddElement(nSubMsgID, xBeginFunctor); }

bool Actor::SendMsg(const ActorMessage &message) { return mMessageQueue.Push(message); }

bool Actor::SendMsg(const int eventID, const std::string &data, const std::string &arg) {
    static ActorMessage xMessage;

    xMessage.id = this->id;
    xMessage.msg_id = eventID;
    xMessage.data = data;
    xMessage.arg = arg;

    return SendMsg(xMessage);
}

bool Actor::BackMsgToMainThread(const ActorMessage &message) { return m_pActorModule->AddResult(message); }

void Actor::ToMemoryCounterString(std::string &info) {
    info.append(id.ToString());
    info.append(":Actor:");
    info.append(std::to_string(mMessageQueue.size_approx()));
}