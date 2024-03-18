#pragma once

#include <squick/core/base.h>
#include <squick/plugin/actor/export.h>
#include <thread>

namespace tutorial {
	class HttpComponent : public IComponent {
	public:
		HttpComponent() : IComponent(typeid(HttpComponent).name()) {}
		virtual ~HttpComponent() {}
		virtual bool Start() {
			AddMsgHandler(0, this, &HttpComponent::OnMsgEvent);
			AddMsgHandler(1, this, &HttpComponent::OnMsgEvent);
			AddMsgHandler(2, this, &HttpComponent::OnMsgEvent);
			AddMsgHandler(3, this, &HttpComponent::OnMsgEvent);
			AddMsgHandler(4, this, &HttpComponent::OnMsgEvent);
			AddMsgHandler(5, this, &HttpComponent::OnMsgEvent);
			AddMsgHandler(6, this, &HttpComponent::OnMsgEvent);
			AddMsgHandler(7, this, &HttpComponent::OnMsgEvent);
			AddMsgHandler(8, this, &HttpComponent::OnMsgEvent);
			AddMsgHandler(9, this, &HttpComponent::OnMsgEvent);
			return true;
		}

		// it's very important to note here:
		// sometimes, the function below not finished but other thread call it at the same time
		// the reason is the main thread post a new message to this actor and the schedulel assigned another thread to take the execution right
		// so, you wouldnot use the data which not thread-safe in this function
		virtual int OnMsgEvent(ActorMessage& arg) {
			std::cout << "Thread: " << std::this_thread::get_id() << " MsgID: " << arg.msg_id << " Data:" << arg.data << std::endl;
			return 0;
		}
	};
	class IActorModule : public IModule {};

	class ActorModule : public IActorModule {
	public:
		ActorModule(IPluginManager* p) {
			is_update_ = true;
			pm_ = p;
		}
		virtual bool Start();
		virtual bool AfterStart();
		virtual bool Update();

		void RequestAsyEnd(ActorMessage& actorMessage);
	private:
		::IActorModule* m_actor_;
	};
}