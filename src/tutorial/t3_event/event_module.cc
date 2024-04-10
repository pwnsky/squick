#include "event_module.h"
#include <struct/struct.h>
namespace tutorial {
    bool EventModule::Start() {
        m_world_ = pm_->FindModule<IWorldModule>();
        m_element_ = pm_->FindModule<IElementModule>();
        m_event_ = pm_->FindModule<::IEventModule>();
        m_schedule_ = pm_->FindModule<IScheduleModule>();
        return true;
    }

    int EventModule::OnEvent(const Guid& self, const int event, const DataList& arg) {
        std::cout << "OnEvent EventID: " << event << " self: " << self.nData64 << " argList: " << arg.Int(0) << " "
            << " " << arg.String(1) << std::endl;

        m_world_->SetPropertyInt(self, "Hello", arg.Int(0));
        m_world_->SetPropertyString(self, "Hello", arg.String(1));
        return 0;
    }

    int EventModule::OnHeartBeat(const Guid& self, const std::string& heartBeat, const float time, const int count) {
        int64_t unNowTime = SquickGetTimeMS();
        std::cout << self.ToString() + " heartBeat: " << time << " Count: " << count << "  TimeDis: " << unNowTime - mLastTime << std::endl;
        mLastTime = unNowTime;
        return 0;
    }

    int EventModule::OnClassCallBackEvent(const Guid& self, const std::string& className, const CLASS_OBJECT_EVENT event, const DataList& arg) {
        std::cout << "OnClassCallBackEvent ClassName: " << className << " ID: " << self.nData64 << " Event: " << event << std::endl;
        if (event == COE_CREATE_HASDATA) {
            m_event_->AddEventCallBack(self, 1, this, &EventModule::OnEvent);
            m_schedule_->AddSchedule(self, "OnHeartBeat", this, &EventModule::OnHeartBeat, 5.0f, 10);
            mLastTime = SquickGetTimeMS();
        }
        return 0;
    }

    int EventModule::OnPropertyCallBackEvent(const Guid& self, const std::string& propertyName, const SquickData& oldVar, const SquickData& newVar,
        const int64_t reason) {
        std::cout << "OnPropertyCallBackEvent Property: " << propertyName << " OldValue: " << oldVar.GetInt() << " NewValue: " << newVar.GetInt() << std::endl;
        return 0;
    }

    int EventModule::OnPropertyStrCallBackEvent(const Guid& self, const std::string& propertyName, const SquickData& oldVar, const SquickData& newVar,
        const int64_t reason) {
        std::cout << "OnPropertyCallBackEvent Property: " << propertyName << " OldValue: " << oldVar.GetString() << " NewValue: " << newVar.GetString()
            << std::endl;
        return 0;
    }

    bool EventModule::AfterStart() {
        std::cout << "Hello, world3, AfterStart" << std::endl;
        m_world_->CreateScene(1);
        m_world_->AddClassCallBack(excel::Player::ThisName(), this, &EventModule::OnClassCallBackEvent);
        m_schedule_->AddSchedule(Guid(), "OnHeartBe22222", this, &EventModule::OnHeartBeat, 6.0f, 10);

        std::shared_ptr<IObject> pObject = m_world_->CreateObject(Guid(0, 10), 1, 0, excel::Player::ThisName(), "", DataList::Empty());
        if (!pObject) {
            return false;
        }

        pObject->GetPropertyManager()->AddProperty(pObject->Self(), "Hello", TDATA_STRING);
        pObject->GetPropertyManager()->AddProperty(pObject->Self(), "World", TDATA_INT);

        pObject->AddPropertyCallBack("Hello", this, &EventModule::OnPropertyStrCallBackEvent);
        pObject->AddPropertyCallBack("World", this, &EventModule::OnPropertyCallBackEvent);

        pObject->SetPropertyString("Hello", "hello,World");
        pObject->SetPropertyInt("World", 1111);

        m_event_->DoEvent(pObject->Self(), 1, DataList() << int(100) << "200");
        return true;
    }
}