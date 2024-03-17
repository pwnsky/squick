
#include "actor_module.h"

namespace tutorial {
bool ActorModule::Start() {
    m_actor_ = pm_->FindModule<::IActorModule>();
    return true;
}

bool ActorModule::AfterStart() {
    int messageCount = 1000000;
    // actor test
    {
        std::cout << "Test for actor mode" << std::endl;
        int64_t timeStart = SquickGetTimeMS();

        auto actorID1 = m_actor_->CreateActor();
        m_actor_->AddComponent<HttpComponent>(actorID1->ID());

        for (int i = 0; i < 5; ++i) {
            m_actor_->AddEndFunc(i, this, &ActorModule::RequestAsyEnd);
        }

        for (int i = 5; i < 10; ++i) {
            m_actor_->AddEndFunc(i, [](ActorMessage& actorMessage) -> void {
                // std::cout << "example 2 AddEndFunc " << actorMessage.id.ToString() << " MSGID: " << actorMessage.msg_id << std::endl;
                });
        }

        for (int i = 0; i < messageCount; ++i) {
            m_actor_->SendMsgToActor(actorID1->ID(), Guid(), i, "test");
            // m_pActorModule->SendMsgToActor(actorID1, i, std::to_string(i*i));
        }

        int64_t timeEnd = SquickGetTimeMS();
        int64_t timeCost = timeEnd - timeStart;
        if (timeCost > 0) {
            std::cout << "end Benchmarks, cost: " << timeCost << "ms for " << messageCount << ", qps: " << (messageCount / timeCost) * 1000 << std::endl;
        }
    }
    return true;
}
void ActorModule::RequestAsyEnd(ActorMessage& actorMessage) {
    // std::cout << "Main thread: " << std::this_thread::get_id() << " Actor: " << actorMessage.id.ToString() << " MsgID: " << actorMessage.msg_id << " Data:"
    // << actorMessage.data << std::endl;
}

bool ActorModule::Update() {

    return true;
}


}