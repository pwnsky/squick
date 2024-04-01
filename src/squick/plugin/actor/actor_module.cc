#include "actor_module.h"

ActorModule::ActorModule(IPluginManager *p) {
    is_update_ = true;
    pm_ = p;

    srand((unsigned)time(NULL));
}

ActorModule::~ActorModule() {}

bool ActorModule::Start() {
    m_thread_pool_ = pm_->FindModule<IThreadPoolModule>();

    return true;
}

bool ActorModule::AfterStart() { return true; }

bool ActorModule::BeforeDestory() {
    mxActorMap.clear();
    return true;
}

bool ActorModule::Destory() { return true; }

bool ActorModule::Update() {
    UpdateEvent();
    UpdateResultEvent();
    return true;
}

std::shared_ptr<IActor> ActorModule::CreateActor() {
    guid_index_++;
    if (guid_index_ > 0x7fffffffff) guid_index_ = 1;
    Guid guid = Guid(SquickGetTimeMS(), 0);
    std::shared_ptr<IActor> pActor = std::shared_ptr<IActor>(new Actor(guid, this));
    mxActorMap.insert(std::map<Guid, std::shared_ptr<IActor>>::value_type(pActor->ID(), pActor));
    return pActor;
}

std::shared_ptr<IActor> ActorModule::GetActor(const Guid nActorIndex) {
    auto it = mxActorMap.find(nActorIndex);
    if (it != mxActorMap.end()) {
        return it->second;
    }
    return nullptr;
}

bool ActorModule::AddResult(const ActorMessage &message) { return mxResultQueue.Push(message); }

bool ActorModule::UpdateEvent() {
    static int64_t lastTime = 0;
    int64_t nowTime = SquickGetTimeMS();
    if (nowTime < lastTime + 10) {
        return false;
    }

    lastTime = nowTime;

    for (auto it : mxActorMap) {
        std::shared_ptr<IActor> pActor = it.second;
        if (pActor) {
            if (test) {
                pActor->Update();
            } else {
                m_thread_pool_->DoAsyncTask(pActor->ID(), "", [pActor](ThreadTask &threadTask) -> void { pActor->Update(); });
            }
        }
    }

    return true;
}

bool ActorModule::UpdateResultEvent() {
    ActorMessage actorMessage;
    while (mxResultQueue.try_dequeue(actorMessage)) {
        ACTOR_PROCESS_FUNCTOR_PTR functorPtr_end = mxEndFunctor.GetElement(actorMessage.msg_id);
        if (functorPtr_end) {
            functorPtr_end->operator()(actorMessage);
        }
    }

    return true;
}

bool ActorModule::SendMsgToActor(const Guid actorIndex, const Guid who, const int eventID, const std::string &data, const std::string &arg) {
    static uint64_t index = 0;
    std::shared_ptr<IActor> pActor = GetActor(actorIndex);
    if (nullptr != pActor) {
        ActorMessage xMessage;
        xMessage.id = who;
        xMessage.index = index++;
        xMessage.data = data;
        xMessage.msg_id = eventID;
        xMessage.arg = arg;
        return this->SendMsgToActor(actorIndex, xMessage);
    }
    return false;
}

bool ActorModule::ReleaseActor(const Guid nActorIndex) {
    auto it = mxActorMap.find(nActorIndex);
    if (it != mxActorMap.end()) {
        mxActorMap.erase(it);
        return true;
    }
    return false;
}

bool ActorModule::AddEndFunc(const int subMessageID, ACTOR_PROCESS_FUNCTOR_PTR functorPtr_end) {
    return mxEndFunctor.AddElement(subMessageID, functorPtr_end);
}

bool ActorModule::SendMsgToActor(const Guid actorIndex, const ActorMessage &message) {
    auto it = mxActorMap.find(actorIndex);
    if (it != mxActorMap.end()) {
        // std::cout << "send message " << message.msg_id << " to " << actorIndex.ToString() << " and msg index is " << message.index << std::endl;
        return it->second->SendMsg(message);
    }
    return false;
}
