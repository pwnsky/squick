#include "lua_script_module.h"
#include "lua_pb_module.h"
#include "plugin.h"
#include <assert.h>

// #include "squick/base/kernel.h"
#define TRY_RUN_GLOBAL_SCRIPT_FUN0(strFuncName)                                                                                                                \
    try {                                                                                                                                                      \
        LuaIntf::LuaRef func(mLuaContext, strFuncName);                                                                                                        \
        func.call<LuaIntf::LuaRef>();                                                                                                                          \
    } catch (LuaIntf::LuaException & e) {                                                                                                                      \
        cout << e.what() << endl;                                                                                                                              \
    }

#define TRY_RUN_GLOBAL_SCRIPT_FUN1(strFuncName, arg1)                                                                                                          \
    try {                                                                                                                                                      \
        LuaIntf::LuaRef func(mLuaContext, strFuncName);                                                                                                        \
        func.call<LuaIntf::LuaRef>(arg1);                                                                                                                      \
    } catch (LuaIntf::LuaException & e) {                                                                                                                      \
        cout << e.what() << endl;                                                                                                                              \
    }

#define TRY_RUN_GLOBAL_SCRIPT_FUN2(strFuncName, arg1, arg2)                                                                                                    \
    try {                                                                                                                                                      \
        LuaIntf::LuaRef func(mLuaContext, strFuncName);                                                                                                        \
        func.call<LuaIntf::LuaRef>(arg1, arg2);                                                                                                                \
    } catch (LuaIntf::LuaException & e) {                                                                                                                      \
        cout << e.what() << endl;                                                                                                                              \
    }

#define TRY_LOAD_SCRIPT_FLE(fileName)                                                                                                                          \
    try {                                                                                                                                                      \
        mLuaContext.doFile(fileName);                                                                                                                          \
    } catch (LuaIntf::LuaException & e) {                                                                                                                      \
        cout << e.what() << endl;                                                                                                                              \
    }

bool LuaScriptModule::Awake() {
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_event_ = pm_->FindModule<IEventModule>();
    m_schedule_ = pm_->FindModule<IScheduleModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_net_ = pm_->FindModule<INetModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_pLuaPBModule = pm_->FindModule<ILuaPBModule>();

    LuaPBModule *p = (LuaPBModule *)(m_pLuaPBModule);
    p->SetLuaState(mLuaContext.state());
    Register();
    scriptPath = "../src/lua";
    std::string strRootFile = scriptPath + "/main.lua";

    TRY_LOAD_SCRIPT_FLE(strRootFile.c_str());

    TRY_RUN_GLOBAL_SCRIPT_FUN1("Main", this);

    TRY_RUN_GLOBAL_SCRIPT_FUN0("Awake");

    return true;
}
// 用于Lua 初始化
bool LuaScriptModule::Start() {

    TRY_RUN_GLOBAL_SCRIPT_FUN0("Start");

    return true;
}
// 用于Lua 初始化
bool LuaScriptModule::AfterStart() {
    TRY_RUN_GLOBAL_SCRIPT_FUN0("AfterStart");
    return true;
}

bool LuaScriptModule::Destory() {
    TRY_RUN_GLOBAL_SCRIPT_FUN0("Destroy");
    return true;
}

bool LuaScriptModule::ReadyUpdate() {
    TRY_RUN_GLOBAL_SCRIPT_FUN0("ReadyUpdate");
    return true;
}

// 这里不对lua进行Update，降低Lua不必要的循环运算
// Lua热重载方式通过http接口来显示触发
bool LuaScriptModule::Update() {
    // Update call per second
    auto now = pm_->GetNowTime();
    if (now - last_update_time_ > 0) {
        last_update_time_ = now;
        TRY_RUN_GLOBAL_SCRIPT_FUN0("Update");
    }
    return true;
}

bool LuaScriptModule::BeforeDestory() {
    TRY_RUN_GLOBAL_SCRIPT_FUN0("BeforeDestory");

    return true;
}

LuaIntf::LuaContext &LuaScriptModule::GetLuaEnv() { return this->mLuaContext; }

Guid LuaScriptModule::CreateObject(const Guid &self, const int sceneID, const int groupID, const std::string &className, const std::string &objectIndex,
                                   const DataList &arg) {
    std::shared_ptr<IObject> xObject = m_kernel_->CreateObject(self, sceneID, groupID, className, objectIndex, arg);
    if (xObject) {
        return xObject->Self();
    }
    return Guid();
}

bool LuaScriptModule::ExistObject(const Guid &self) { return m_kernel_->ExistObject(self); }

bool LuaScriptModule::DestroyObject(const Guid &self) { return m_kernel_->DestroyObject(self); }

bool LuaScriptModule::EnterScene(const int sceneID, const int groupID) { return false; }

bool LuaScriptModule::DoEvent(const Guid &self, const int eventID, const DataList &arg) {
    m_event_->DoEvent(self, (int)eventID, arg);

    return true;
}

bool LuaScriptModule::FindProperty(const Guid &self, const std::string &propertyName) { return m_kernel_->FindProperty(self, propertyName); }

bool LuaScriptModule::SetPropertyInt(const Guid &self, const std::string &propertyName, const INT64 propValue) {
    return m_kernel_->SetPropertyInt(self, propertyName, propValue);
}

bool LuaScriptModule::SetPropertyFloat(const Guid &self, const std::string &propertyName, const double propValue) {
    return m_kernel_->SetPropertyFloat(self, propertyName, propValue);
}

bool LuaScriptModule::SetPropertyString(const Guid &self, const std::string &propertyName, const std::string &propValue) {
    return m_kernel_->SetPropertyString(self, propertyName, propValue);
}

bool LuaScriptModule::SetPropertyObject(const Guid &self, const std::string &propertyName, const Guid &propValue) {
    return m_kernel_->SetPropertyObject(self, propertyName, propValue);
}

bool LuaScriptModule::SetPropertyVector2(const Guid &self, const std::string &propertyName, const Vector2 &propValue) {
    return m_kernel_->SetPropertyVector2(self, propertyName, propValue);
}

bool LuaScriptModule::SetPropertyVector3(const Guid &self, const std::string &propertyName, const Vector3 &propValue) {
    return m_kernel_->SetPropertyVector3(self, propertyName, propValue);
}

INT64 LuaScriptModule::GetPropertyInt(const Guid &self, const std::string &propertyName) { return m_kernel_->GetPropertyInt(self, propertyName); }

int LuaScriptModule::GetPropertyInt32(const Guid &self, const std::string &propertyName) { return m_kernel_->GetPropertyInt32(self, propertyName); }

double LuaScriptModule::GetPropertyFloat(const Guid &self, const std::string &propertyName) { return m_kernel_->GetPropertyFloat(self, propertyName); }

std::string LuaScriptModule::GetPropertyString(const Guid &self, const std::string &propertyName) { return m_kernel_->GetPropertyString(self, propertyName); }

Guid LuaScriptModule::GetPropertyObject(const Guid &self, const std::string &propertyName) { return m_kernel_->GetPropertyObject(self, propertyName); }

Vector2 LuaScriptModule::GetPropertyVector2(const Guid &self, const std::string &propertyName) { return m_kernel_->GetPropertyVector2(self, propertyName); }

Vector3 LuaScriptModule::GetPropertyVector3(const Guid &self, const std::string &propertyName) { return m_kernel_->GetPropertyVector3(self, propertyName); }

bool LuaScriptModule::AddClassCallBack(std::string &className, const LuaIntf::LuaRef &luaTable, const LuaIntf::LuaRef &luaFunc) {
    auto callbackList = mxClassEventFuncMap.GetElement(className);
    if (!callbackList) {
        callbackList = new List<LuaCallBack>();
        mxClassEventFuncMap.AddElement(className, callbackList);
        m_kernel_->AddClassCallBack(className, this, &LuaScriptModule::OnClassEventCB);
    }

    LuaCallBack callback = { luaTable };
    callbackList->Add(callback);

    return false;
}

int LuaScriptModule::OnClassEventCB(const Guid &objectId, const std::string &className, const CLASS_OBJECT_EVENT classEvent, const DataList &var) {
    auto funcNameList = mxClassEventFuncMap.GetElement(className);
    if (funcNameList) {
        LuaCallBack callback;
        bool ret = funcNameList->First(callback);
        while (ret) {
            try {
                callback.func.call(callback.self, objectId, className, (int)classEvent, (DataList)var);
            } catch (LuaIntf::LuaException &e) {
                cout << e.what() << endl;
                return 0;
            } catch (...) {
                return 0;
            }

            ret = funcNameList->Next(callback);
        }
    }

    return -1;
}

void LuaScriptModule::OnScriptReload() {
    std::string strRootFile = scriptPath + "/reload.lua";
    if (!strRootFile.empty()) {
        TRY_LOAD_SCRIPT_FLE(strRootFile.c_str());
    }
}

bool LuaScriptModule::AddPropertyCallBack(const Guid &self, std::string &propertyName, const LuaIntf::LuaRef &luaTable, const LuaIntf::LuaRef &luaFunc) {
    LuaCallBack callback = { luaTable };
    if (AddLuaFuncToMap(mxLuaPropertyCallBackFuncMap, self, propertyName, callback)) {
        m_kernel_->AddPropertyCallBack(self, propertyName, this, &LuaScriptModule::OnLuaPropertyCB);
    }
    return true;
}

int LuaScriptModule::OnLuaPropertyCB(const Guid &self, const std::string &propertyName, const SquickData &oldVar, const SquickData &newVar,
                                     const INT64 reason) {
    auto funcList = mxLuaPropertyCallBackFuncMap.GetElement(propertyName);
    if (funcList) {
        auto funcNameList = funcList->GetElement(self);
        if (funcNameList) {
            LuaCallBack callback;
            auto Ret = funcNameList->First(callback);
            while (Ret) {
                try {
                    callback.func.call(callback.self, self, propertyName, oldVar, newVar);
                } catch (LuaIntf::LuaException &e) {
                    cout << e.what() << endl;
                } catch (...) {
                }

                Ret = funcNameList->Next(callback);
            }
        }
    }

    return 0;
}

bool LuaScriptModule::AddRecordCallBack(const Guid &self, std::string &recordName, const LuaIntf::LuaRef &luaTable, const LuaIntf::LuaRef &luaFunc) {
    LuaCallBack callback = { luaTable, luaFunc };
    if (AddLuaFuncToMap(mxLuaRecordCallBackFuncMap, self, recordName, callback)) {
        m_kernel_->AddRecordCallBack(self, recordName, this, &LuaScriptModule::OnLuaRecordCB);
    }
    return true;
}

int LuaScriptModule::OnLuaRecordCB(const Guid &self, const RECORD_EVENT_DATA &eventData, const SquickData &oldVar, const SquickData &newVar) {
    auto funcList = mxLuaRecordCallBackFuncMap.GetElement(eventData.recordName);
    if (funcList) {
        auto funcNameList = funcList->GetElement(self);
        if (funcNameList) {
            LuaCallBack callback;
            auto Ret = funcNameList->First(callback);
            while (Ret) {
                try {
                    callback.func.call<LuaIntf::LuaRef>("", self, eventData.recordName, eventData.nOpType, eventData.row, eventData.col, oldVar, newVar);
                } catch (LuaIntf::LuaException &e) {
                    cout << e.what() << endl;
                } catch (...) {
                }

                Ret = funcNameList->Next(callback);
            }
        }
    }

    return 0;
}

bool LuaScriptModule::AddEventCallBack(const Guid &self, const int eventID, const LuaIntf::LuaRef &luaTable, const LuaIntf::LuaRef &luaFunc) {
    LuaCallBack callback = { luaTable, luaFunc };
    if (AddLuaFuncToMap(mxLuaEventCallBackFuncMap, self, (int)eventID, callback)) {
        m_event_->AddEventCallBack(self, eventID, this, &LuaScriptModule::OnLuaEventCB);
    }
    return true;
}

int LuaScriptModule::OnLuaEventCB(const Guid &self, const int eventID, const DataList &argVar) {

    auto funcList = mxLuaEventCallBackFuncMap.GetElement(eventID);
    if (funcList) {
        auto funcNameList = funcList->GetElement(self);
        if (funcNameList) {
            LuaCallBack callback;
            auto Ret = funcNameList->First(callback);
            while (Ret) {
                try {
                    callback.func.call<LuaIntf::LuaRef>(callback.self, self, eventID, (DataList &)argVar);
                } catch (LuaIntf::LuaException &e) {
                    cout << e.what() << endl;
                } catch (...) {
                }

                Ret = funcNameList->Next(callback);
            }
        }
    }

    return 0;
}

bool LuaScriptModule::AddModuleSchedule(std::string &strHeartBeatName, const LuaIntf::LuaRef &luaTable, const LuaIntf::LuaRef &luaFunc, const float time,
                                        const int count) {
    LuaCallBack callback = { luaTable, luaFunc };
    if (AddLuaFuncToMap(mxLuaHeartBeatCallBackFuncMap, strHeartBeatName, callback)) {
        return m_schedule_->AddSchedule(Guid(), strHeartBeatName, this, &LuaScriptModule::OnLuaHeartBeatCB, time, count);
    }

    return false;
}

bool LuaScriptModule::AddSchedule(const Guid &self, std::string &strHeartBeatName, const LuaIntf::LuaRef &luaTable, const LuaIntf::LuaRef &luaFunc,
                                  const float time, const int count) {
    LuaCallBack callback = { luaTable, luaFunc };
    if (AddLuaFuncToMap(mxLuaHeartBeatCallBackFuncMap, self, strHeartBeatName, callback)) {
        m_schedule_->AddSchedule(self, strHeartBeatName, this, &LuaScriptModule::OnLuaHeartBeatCB, time, count);
    }
    return true;
}

int LuaScriptModule::OnLuaHeartBeatCB(const Guid &self, const std::string &strHeartBeatName, const float time, const int count) {

    auto funcList = mxLuaHeartBeatCallBackFuncMap.GetElement(strHeartBeatName);
    if (funcList) {
        auto funcNameList = funcList->GetElement(self);
        if (funcNameList) {
            LuaCallBack callback;
            auto Ret = funcNameList->First(callback);
            while (Ret) {
                try {
                    callback.func.call<LuaIntf::LuaRef>(callback.self, self, strHeartBeatName, time, count);
                } catch (LuaIntf::LuaException &e) {
                    cout << e.what() << endl;
                } catch (...) {
                }

                Ret = funcNameList->Next(callback);
            }
        }
    }

    return 0;
}

int LuaScriptModule::AddRow(const Guid &self, std::string &recordName, const DataList &var) {
    std::shared_ptr<IRecord> pRecord = m_kernel_->FindRecord(self, recordName);
    if (nullptr == pRecord) {
        return -1;
    }

    return pRecord->AddRow(-1, var);
}

bool LuaScriptModule::RemRow(const Guid &self, std::string &recordName, const int row) {
    std::shared_ptr<IRecord> pRecord = m_kernel_->FindRecord(self, recordName);
    if (nullptr == pRecord) {
        return false;
    }

    return pRecord->Remove(row);
}

bool LuaScriptModule::SetRecordInt(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const INT64 value) {
    return m_kernel_->SetRecordInt(self, recordName, row, colTag, value);
}

bool LuaScriptModule::SetRecordFloat(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const double value) {
    return m_kernel_->SetRecordFloat(self, recordName, row, colTag, value);
}

bool LuaScriptModule::SetRecordString(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const std::string &value) {
    return m_kernel_->SetRecordString(self, recordName, row, colTag, value);
}

bool LuaScriptModule::SetRecordObject(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const Guid &value) {
    return m_kernel_->SetRecordObject(self, recordName, row, colTag, value);
}

bool LuaScriptModule::SetRecordVector2(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const Vector2 &value) {
    return m_kernel_->SetRecordVector2(self, recordName, row, colTag, value);
}

bool LuaScriptModule::SetRecordVector3(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const Vector3 &value) {
    return m_kernel_->SetRecordVector3(self, recordName, row, colTag, value);
}

INT64 LuaScriptModule::GetRecordInt(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    return m_kernel_->GetRecordInt(self, recordName, row, colTag);
}

double LuaScriptModule::GetRecordFloat(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    return m_kernel_->GetRecordFloat(self, recordName, row, colTag);
}

std::string LuaScriptModule::GetRecordString(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    return m_kernel_->GetRecordString(self, recordName, row, colTag);
}

Guid LuaScriptModule::GetRecordObject(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    return m_kernel_->GetRecordObject(self, recordName, row, colTag);
}

Vector2 LuaScriptModule::GetRecordVector2(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    return m_kernel_->GetRecordVector2(self, recordName, row, colTag);
}

Vector3 LuaScriptModule::GetRecordVector3(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    return m_kernel_->GetRecordVector3(self, recordName, row, colTag);
}

INT64 LuaScriptModule::GetNowTime() { return pm_->GetNowTime(); }

Guid LuaScriptModule::CreateID() { return m_kernel_->CreateGUID(); }

int LuaScriptModule::AppID() { return pm_->GetAppID(); }

int LuaScriptModule::AppType() {
    return pm_->GetAppType();
}

string LuaScriptModule::AppName() {
    return pm_->GetAppName();
}

INT64 LuaScriptModule::Area() {
    return pm_->GetArea();
}

bool LuaScriptModule::ExistElementObject(const std::string &configName) { return m_element_->ExistElement(configName); }

LuaIntf::LuaRef LuaScriptModule::GetConfigIDList(const string& className) {
    LuaIntf::LuaRef tbl = LuaIntf::LuaRef::createTable(mLuaContext);
    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(className);
    if (xLogicClass) {
        auto list = xLogicClass->GetIDList();
        int i = 1;
        for (auto& id : list) {
            tbl[i] = id;
            i++;
        }
    }
    return tbl;
}

LuaIntf::LuaRef LuaScriptModule::GetConfig(const string& className) {
    LuaIntf::LuaRef ret = LuaIntf::LuaRef::createTable(mLuaContext);
    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(className);
    if (xLogicClass) {
        auto list = xLogicClass->GetIDList();
        for (auto& id : list) {
            ret[id] = GetConfigByID(id);
        }
    }
    return ret;
}

LuaIntf::LuaRef LuaScriptModule::GetConfigByID(const string& id) {
	LuaIntf::LuaRef ret = LuaIntf::LuaRef::createTable(mLuaContext);
	auto m = m_element_->GetPropertyManager(id);
	string key;
	auto p = m->First(key);
	while (p) {
		switch (p->GetType()) {
		case TDATA_INT:
            ret[key] = p->GetInt32();
			break;
		case TDATA_FLOAT:
            ret[key] = p->GetFloat();
			break;
		case TDATA_STRING:
            ret[key] = p->GetString();
			break;
		case TDATA_OBJECT:
            ret[key] = p->GetString();
			break;
		case TDATA_VECTOR2:
            ret[key] = p->GetVector2().ToString();
			break;
		case TDATA_VECTOR3:
            ret[key] = p->GetVector3().ToString();
			break;
		default:
            ret[key] = NULL_STR;
			break;
		}
		p = m->Next(key);
	}
    return ret;
}

INT64 LuaScriptModule::GetElePropertyInt(const std::string &configName, const std::string &propertyName) {
    return m_element_->GetPropertyInt(configName, propertyName);
}

double LuaScriptModule::GetElePropertyFloat(const std::string &configName, const std::string &propertyName) {
    return m_element_->GetPropertyFloat(configName, propertyName);
}

std::string LuaScriptModule::GetElePropertyString(const std::string &configName, const std::string &propertyName) {
    return m_element_->GetPropertyString(configName, propertyName);
}

Vector2 LuaScriptModule::GetElePropertyVector2(const std::string &configName, const std::string &propertyName) {
    return m_element_->GetPropertyVector2(configName, propertyName);
}

Vector3 LuaScriptModule::GetElePropertyVector3(const std::string &configName, const std::string &propertyName) {
    return m_element_->GetPropertyVector3(configName, propertyName);
}

template <typename T>
bool LuaScriptModule::AddLuaFuncToMap(Map<T, Map<Guid, List<LuaCallBack>>> &funcMap, const Guid &self, T key, const LuaCallBack &callback) {
    auto funcList = funcMap.GetElement(key);
    if (!funcList) {
        List<LuaCallBack> *funcNameList = new List<LuaCallBack>;
        funcNameList->Add(callback);
        funcList = new Map<Guid, List<LuaCallBack>>;
        funcList->AddElement(self, funcNameList);
        funcMap.AddElement(key, funcList);
        return true;
    }

    if (!funcList->GetElement(self)) {
        List<LuaCallBack> *funcNameList = new List<LuaCallBack>;
        funcNameList->Add(callback);
        funcList->AddElement(self, funcNameList);
        return true;
    } else {
        auto funcNameList = funcList->GetElement(self);
        funcNameList->Add(callback);
        return true;
    }

    return false;
}

template <typename T> bool LuaScriptModule::AddLuaFuncToMap(Map<T, Map<Guid, List<LuaCallBack>>> &funcMap, T key, const LuaCallBack &callback) {
    auto funcList = funcMap.GetElement(key);
    if (!funcList) {
        List<LuaCallBack> *funcNameList = new List<LuaCallBack>;
        funcNameList->Add(callback);
        funcList = new Map<Guid, List<LuaCallBack>>;
        funcList->AddElement(Guid(), funcNameList);
        funcMap.AddElement(key, funcList);
        return true;
    }

    if (!funcList->GetElement(Guid())) {
        List<LuaCallBack> *funcNameList = new List<LuaCallBack>;
        funcNameList->Add(callback);
        funcList->AddElement(Guid(), funcNameList);
        return true;
    } else {
        auto funcNameList = funcList->GetElement(Guid());
        funcNameList->Add(callback);
        return true;
    }

    return false;
}

void LuaScriptModule::RemoveCallBackAsServer(const int msg_id) { m_net_->RemoveReceiveCallBack(msg_id); }

void LuaScriptModule::AddMsgCallBackAsServer(const int msg_id, const LuaIntf::LuaRef &luaTable, const LuaIntf::LuaRef &luaFunc) {
    auto callbackList = mxNetMsgCallBackFuncMapAsServer.GetElement(msg_id);
    if (!callbackList) {
        callbackList = new List<LuaCallBack>();
        mxNetMsgCallBackFuncMapAsServer.AddElement(msg_id, callbackList);
        m_net_->AddReceiveCallBack(msg_id, this, &LuaScriptModule::OnNetMsgCallBackAsServer);
    }
    callbackList->Add({ luaTable, luaFunc });
}

void LuaScriptModule::RemoveMsgCallBackAsClient(const ServerType serverType, const int msg_id) { m_net_client_->RemoveReceiveCallBack(serverType, msg_id); }

// 做为服务器做为客户端连接的网络 回调
void LuaScriptModule::AddMsgCallBackAsClient(const ServerType serverType, const int msg_id, const LuaIntf::LuaRef &luaTable, const LuaIntf::LuaRef &luaFunc) {
    auto callbackList = mxNetMsgCallBackFuncMapAsClient.GetElement(msg_id);
    if (!callbackList) {
        callbackList = new List<LuaCallBack>();
        mxNetMsgCallBackFuncMapAsClient.AddElement(msg_id, callbackList);
        m_net_client_->AddReceiveCallBack(serverType, msg_id, this, &LuaScriptModule::OnNetMsgCallBackAsClient);
    }
    callbackList->Add({ luaTable, luaFunc });
    
}

bool LuaScriptModule::ImportProtoFile(const std::string &fileName) {
    LuaPBModule *p = (LuaPBModule *)m_pLuaPBModule;
    return p->ImportProtoFile(fileName);
}

const std::string LuaScriptModule::Encode(const std::string &msgTypeName, const LuaIntf::LuaRef &luaTable) {
    LuaPBModule *p = (LuaPBModule *)m_pLuaPBModule;
    return p->Encode(msgTypeName, luaTable);
}

LuaIntf::LuaRef LuaScriptModule::Decode(const std::string &msgTypeName, const std::string &data) {
    LuaPBModule *p = (LuaPBModule *)m_pLuaPBModule;
    return p->Decode(msgTypeName, data);
}

void LuaScriptModule::SetScriptPath(const std::string &path) { scriptPath = path; }

const std::string LuaScriptModule::GetScriptPath() { return scriptPath; }

void LuaScriptModule::SendToServerByServerID(const int server_id, const uint16_t msg_id, const std::string &data, const uint64_t uid) {
    if (pm_->GetAppID() == server_id) {
        m_log_->LogError("you can send message to yourself");
        return;
    }
    m_net_client_->SendByID(server_id, msg_id, data, uid);
}

void LuaScriptModule::SendToAllServerByServerType(const ServerType server_type, const uint16_t msg_id, const std::string &data, const uint64_t uid) {
    m_net_client_->SendToAllNodeByType(server_type, msg_id, data, uid);
}

void LuaScriptModule::SendByFD(const socket_t fd, const uint16_t msg_id, const std::string &data, const uint64_t uid) {
    m_net_->SendMsg(msg_id, data, fd, uid);
}

void LuaScriptModule::LogInfo(const std::string &logData) { m_log_->LogInfo(logData); }

void LuaScriptModule::LogError(const std::string &logData) { m_log_->LogError(logData); }

void LuaScriptModule::LogWarning(const std::string &logData) { m_log_->LogWarning(logData); }

void LuaScriptModule::LogDebug(const std::string &logData) { m_log_->LogDebug(logData); }

void LuaScriptModule::SetVersionCode(const std::string &logData) { strVersionCode = logData; }

const std::string &LuaScriptModule::GetVersionCode() { return strVersionCode; }

bool LuaScriptModule::Register() {
    LuaIntf::LuaBinding(mLuaContext)
        .beginClass<Guid>("Guid")
        .addConstructor(LUA_ARGS())
        .addProperty("GetData", &Guid::GetData, &Guid::SetData)
        .addProperty("GetHead", &Guid::GetHead, &Guid::SetHead)
        .addFunction("ToString", &Guid::ToString)
        .addFunction("FromString", &Guid::FromString)
        .endClass();

    LuaIntf::LuaBinding(mLuaContext)
        .beginClass<Vector3>("Vector3")
        .addConstructor(LUA_ARGS())
        .addFunction("FromString", &Vector3::FromString)
        .addFunction("ToString", &Vector3::ToString)
        .addFunction("X", &Vector3::X)
        .addFunction("Y", &Vector3::Y)
        .addFunction("Z", &Vector3::Z)
        .endClass();

    LuaIntf::LuaBinding(mLuaContext)
        .beginClass<Vector2>("Vector2")
        .addConstructor(LUA_ARGS())
        .addFunction("FromString", &Vector2::FromString)
        .addFunction("ToString", &Vector2::ToString)
        .addFunction("X", &Vector2::X)
        .addFunction("Y", &Vector2::Y)
        .endClass();

    LuaIntf::LuaBinding(mLuaContext).beginClass<DataList>("DataList").endClass();

    LuaIntf::LuaBinding(mLuaContext)
        .beginExtendClass<DataList, DataList>("DataList")
        .addConstructor(LUA_ARGS())
        .addFunction("IsEmpty", &DataList::IsEmpty)
        .addFunction("GetCount", &DataList::GetCount)
        .addFunction("Type", &DataList::Type)

        .addFunction("AddInt", &DataList::AddInt)
        .addFunction("AddFloat", &DataList::AddFloat)
        .addFunction("AddStringFromChar", &DataList::AddStringFromChar)
        .addFunction("AddObject", &DataList::AddObject)
        .addFunction("AddVector2", &DataList::AddVector2)
        .addFunction("AddVector3", &DataList::AddVector3)

        .addFunction("SetInt", &DataList::SetInt)
        .addFunction("SetFloat", &DataList::SetFloat)
        .addFunction("SetString", &DataList::SetString)
        .addFunction("SetObject", &DataList::SetObject)
        .addFunction("SetVector2", &DataList::SetVector2)
        .addFunction("SetVector3", &DataList::SetVector3)

        .addFunction("Int", &DataList::Int)
        .addFunction("Float", &DataList::Float)
        .addFunction("String", &DataList::String)
        .addFunction("Object", &DataList::Object)
        .addFunction("Vector2At", &DataList::Vector2At)
        .addFunction("Vector3At", &DataList::Vector3At)
        .endClass();

    LuaIntf::LuaBinding(mLuaContext)
        .beginClass<SquickData>("TData")
        .addConstructor(LUA_ARGS())
        .addFunction("GetFloat", &SquickData::GetFloat)
        .addFunction("GetInt", &SquickData::GetInt)
        .addFunction("GetObject", &SquickData::GetObject)
        .addFunction("GetString", &SquickData::GetString)
        .addFunction("GetVector2", &SquickData::GetVector2)
        .addFunction("GetVector3", &SquickData::GetVector3)

        .addFunction("GetType", &SquickData::GetType)
        .addFunction("IsNullValue", &SquickData::IsNullValue)

        .addFunction("SetFloat", &SquickData::SetFloat)
        .addFunction("SetInt", &SquickData::SetInt)
        .addFunction("SetObject", &SquickData::SetObject)
        .addFunction("SetString", &SquickData::SetString)
        .addFunction("SetVector2", &SquickData::SetVector2)
        .addFunction("SetVector3", &SquickData::SetVector3)
        .endClass();

    

    // for kernel module
    LuaIntf::LuaBinding(mLuaContext)
        .beginClass<LuaScriptModule>("LuaScriptModule")
        .addFunction("CreateObject", &LuaScriptModule::CreateObject)
        .addFunction("ExistObject", &LuaScriptModule::ExistObject)
        .addFunction("DestroyObject", &LuaScriptModule::DestroyObject)
        .addFunction("EnterScene", &LuaScriptModule::EnterScene)
        .addFunction("DoEvent", &LuaScriptModule::DoEvent)

        .addFunction("SetPropertyInt", &LuaScriptModule::SetPropertyInt)
        .addFunction("SetPropertyFloat", &LuaScriptModule::SetPropertyFloat)
        .addFunction("SetPropertyString", &LuaScriptModule::SetPropertyString)
        .addFunction("SetPropertyObject", &LuaScriptModule::SetPropertyObject)
        .addFunction("SetPropertyVector2", &LuaScriptModule::SetPropertyVector2)
        .addFunction("SetPropertyVector3", &LuaScriptModule::SetPropertyVector3)

        .addFunction("GetPropertyInt", &LuaScriptModule::GetPropertyInt)
        .addFunction("GetPropertyFloat", &LuaScriptModule::GetPropertyFloat)
        .addFunction("GetPropertyString", &LuaScriptModule::GetPropertyString)
        .addFunction("GetPropertyObject", &LuaScriptModule::GetPropertyObject)
        .addFunction("GetPropertyVector2", &LuaScriptModule::GetPropertyVector2)
        .addFunction("GetPropertyVector3", &LuaScriptModule::GetPropertyVector3)

        .addFunction("SetRecordInt", &LuaScriptModule::SetRecordInt)
        .addFunction("SetRecordFloat", &LuaScriptModule::SetRecordFloat)
        .addFunction("SetRecordString", &LuaScriptModule::SetRecordString)
        .addFunction("SetRecordObject", &LuaScriptModule::SetRecordObject)
        .addFunction("SetPropertyVector2", &LuaScriptModule::SetPropertyVector2)
        .addFunction("SetPropertyVector3", &LuaScriptModule::SetPropertyVector3)

        .addFunction("GetRecordInt", &LuaScriptModule::GetRecordInt)
        .addFunction("GetRecordFloat", &LuaScriptModule::GetRecordFloat)
        .addFunction("GetRecordString", &LuaScriptModule::GetRecordString)
        .addFunction("GetRecordObject", &LuaScriptModule::GetRecordObject)
        .addFunction("GetPropertyVector2", &LuaScriptModule::GetPropertyVector2)
        .addFunction("GetPropertyVector3", &LuaScriptModule::GetPropertyVector3)

        .addFunction("AddPropertyCallBack", &LuaScriptModule::AddPropertyCallBack)
        .addFunction("AddRecordCallBack", &LuaScriptModule::AddRecordCallBack)
        .addFunction("AddEventCallBack", &LuaScriptModule::AddEventCallBack)
        .addFunction("AddClassCallBack", &LuaScriptModule::AddClassCallBack)
        .addFunction("AddSchedule", &LuaScriptModule::AddSchedule)
        .addFunction("AddModuleSchedule", &LuaScriptModule::AddModuleSchedule)
        .addFunction("DoEvent", &LuaScriptModule::DoEvent)
        .addFunction("AddRow", &LuaScriptModule::AddRow)
        .addFunction("RemRow", &LuaScriptModule::RemRow)

        .addFunction("GetNowTime", &LuaScriptModule::GetNowTime)
        .addFunction("CreateID", &LuaScriptModule::CreateID)
        .addFunction("AppID", &LuaScriptModule::AppID)
        .addFunction("AppType", &LuaScriptModule::AppType)
        .addFunction("AppName", &LuaScriptModule::AppName)
        .addFunction("Area", &LuaScriptModule::Area)

        // Config
        .addFunction("ExistElementObject", &LuaScriptModule::ExistElementObject)
        .addFunction("GetConfigIDList", &LuaScriptModule::GetConfigIDList)
        .addFunction("GetConfig", &LuaScriptModule::GetConfig)
        .addFunction("GetConfigByID", &LuaScriptModule::GetConfigByID)
        .addFunction("GetElePropertyInt", &LuaScriptModule::GetElePropertyInt)
        .addFunction("GetElePropertyFloat", &LuaScriptModule::GetElePropertyFloat)
        .addFunction("GetElePropertyString", &LuaScriptModule::GetElePropertyString)
        .addFunction("GetElePropertyVector2", &LuaScriptModule::GetElePropertyVector2)
        .addFunction("GetElePropertyVector3", &LuaScriptModule::GetElePropertyVector3)

        // 网络模块绑定
        .addFunction("RemoveCallBackAsServer", &LuaScriptModule::RemoveCallBackAsServer)    // as server
        .addFunction("AddMsgCallBackAsServer", &LuaScriptModule::AddMsgCallBackAsServer)       // as server
        .addFunction("RemoveMsgCallBackAsClient", &LuaScriptModule::RemoveMsgCallBackAsClient) // as client
        .addFunction("AddMsgCallBackAsClient", &LuaScriptModule::AddMsgCallBackAsClient)       // as client

        .addFunction("SendToServerByServerID", &LuaScriptModule::SendToServerByServerID)           // as client
        .addFunction("SendToAllServerByServerType", &LuaScriptModule::SendToAllServerByServerType) // as client
        .addFunction("SendByFD", &LuaScriptModule::SendByFD) // as server

        // Log
        .addFunction("LogInfo", &LuaScriptModule::LogInfo)
        .addFunction("LogError", &LuaScriptModule::LogError)
        .addFunction("LogWarning", &LuaScriptModule::LogWarning)
        .addFunction("LogDebug", &LuaScriptModule::LogDebug)

        // Version
        .addFunction("GetVersionCode", &LuaScriptModule::GetVersionCode)
        .addFunction("SetVersionCode", &LuaScriptModule::SetVersionCode)

        // Proto
        .addFunction("ImportProto", &LuaScriptModule::ImportProtoFile)
        .addFunction("Encode", &LuaScriptModule::Encode)
        .addFunction("Decode", &LuaScriptModule::Decode)
        .addFunction("GetScriptPath", &LuaScriptModule::GetScriptPath)
        .endClass();

    return true;
}

void LuaScriptModule::OnNetMsgCallBackAsServer(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    uint64_t uid;
    string data;
    if (!m_net_->ReceivePB(msg_id, msg, len, data, uid)) {
        ostringstream str;
        str << "Parse Message Failed from Packet to MsgBase! MessageID: " << msg_id;
        m_log_->LogWarning(str);
        return;
    }
    auto msgCallBack = mxNetMsgCallBackFuncMapAsServer.GetElement(msg_id);
    if (msgCallBack) {
        LuaCallBack callback;
        auto Ret = msgCallBack->First(callback);
        while (Ret) {
            try {
                callback.func.call<LuaIntf::LuaRef>(callback.self, uid, data, msg_id, sock);
            } catch (LuaIntf::LuaException &e) {
                cout << e.what() << endl;
            } catch (...) {
            }
            Ret = msgCallBack->Next(callback);
        }
    }
}

void LuaScriptModule::OnNetMsgCallBackAsClient(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    uint64_t uid;
    string data;
    if (!m_net_->ReceivePB(msg_id, msg, len, data, uid)) {
        ostringstream str;
        str << "Parse Message Failed from Packet to MsgBase! MessageID: " << msg_id;
        m_log_->LogWarning(str);
        return;
    }
	auto msgCallBack = mxNetMsgCallBackFuncMapAsClient.GetElement(msg_id);
	if (msgCallBack) {
		LuaCallBack callback;
		auto Ret = msgCallBack->First(callback);
		while (Ret) {
			try {
				callback.func.call<LuaIntf::LuaRef>(callback.self, uid, data, msg_id, sock);
			}
			catch (LuaIntf::LuaException& e) {
				cout << e.what() << endl;
			}
			catch (...) {
			}
			Ret = msgCallBack->Next(callback);
		}
	}
}