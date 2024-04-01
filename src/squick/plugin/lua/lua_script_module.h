#pragma once

#ifdef min
#undef min
#endif

// just define it as 0 if you want to use luaintf with C
// #define LUAINTF_LINK_LUA_COMPILED_IN_CXX 0

#include "lua_pb_module.h"
#include <squick/plugin/config/export.h>
#include <squick/plugin/utils/export.h>
#include <squick/plugin/world/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>


#if PLATFORM == PLATFORM_WIN
#elif PLATFORM == PLATFORM_LINUX
#include <sys/inotify.h>
#endif

#include <fcntl.h>
#include <limits.h>

class ILuaScriptModule : public IModule {
  public:
};

class LuaScriptModule : public ILuaScriptModule {
    struct LuaCallBack {
        LuaIntf::LuaRef self;
        LuaIntf::LuaRef func;
    };

  public:
    LuaScriptModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
    }

    virtual bool Awake();
    virtual bool Start();
    virtual bool Destory();
    virtual bool ReadyUpdate();
    virtual bool Update();

    virtual bool AfterStart();
    virtual bool BeforeDestory();

    virtual LuaIntf::LuaContext &GetLuaEnv();

  protected:

    // FOR KERNEL MODULE
    Guid CreateObject(const Guid &self, const int sceneID, const int groupID, const std::string &className, const std::string &objectIndex,
                      const DataList &arg);
    bool ExistObject(const Guid &self);
    bool DestroyObject(const Guid &self);

    // return the group id
    bool EnterScene(const int sceneID, const int groupID);
    bool DoEvent(const Guid &self, const int eventID, const DataList &arg);

    bool FindProperty(const Guid &self, const std::string &propertyName);

    bool SetPropertyInt(const Guid &self, const std::string &propertyName, const INT64 propValue);
    bool SetPropertyFloat(const Guid &self, const std::string &propertyName, const double propValue);
    bool SetPropertyString(const Guid &self, const std::string &propertyName, const std::string &propValue);
    bool SetPropertyObject(const Guid &self, const std::string &propertyName, const Guid &propValue);
    bool SetPropertyVector2(const Guid &self, const std::string &propertyName, const Vector2 &propValue);
    bool SetPropertyVector3(const Guid &self, const std::string &propertyName, const Vector3 &propValue);

    INT64 GetPropertyInt(const Guid &self, const std::string &propertyName);
    int GetPropertyInt32(const Guid &self, const std::string &propertyName); // equal to (int)GetPropertyInt(...), to remove C4244 warning
    double GetPropertyFloat(const Guid &self, const std::string &propertyName);
    std::string GetPropertyString(const Guid &self, const std::string &propertyName);
    Guid GetPropertyObject(const Guid &self, const std::string &propertyName);
    Vector2 GetPropertyVector2(const Guid &self, const std::string &propertyName);
    Vector3 GetPropertyVector3(const Guid &self, const std::string &propertyName);

    bool AddPropertyCallBack(const Guid &self, std::string &propertyName, const LuaIntf::LuaRef &luaTable, const LuaIntf::LuaRef &luaFunc);
    bool AddRecordCallBack(const Guid &self, std::string &recordName, const LuaIntf::LuaRef &luaTable, const LuaIntf::LuaRef &luaFunc);
    bool AddEventCallBack(const Guid &self, const int eventID, const LuaIntf::LuaRef &luaTable, const LuaIntf::LuaRef &luaFunc);
    bool AddSchedule(const Guid &self, std::string &strHeartBeatName, const LuaIntf::LuaRef &luaTable, const LuaIntf::LuaRef &luaFunc, const float time,
                     const int count);
    bool AddModuleSchedule(std::string &strHeartBeatName, const LuaIntf::LuaRef &luaTable, const LuaIntf::LuaRef &luaFunc, const float time, const int count);

    int AddRow(const Guid &self, std::string &recordName, const DataList &var);
    bool RemRow(const Guid &self, std::string &recordName, const int row);

    bool SetRecordInt(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const INT64 value);
    bool SetRecordFloat(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const double value);
    bool SetRecordString(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const std::string &value);
    bool SetRecordObject(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const Guid &value);
    bool SetRecordVector2(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const Vector2 &value);
    bool SetRecordVector3(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const Vector3 &value);

    INT64 GetRecordInt(const Guid &self, const std::string &recordName, const int row, const std::string &colTag);
    double GetRecordFloat(const Guid &self, const std::string &recordName, const int row, const std::string &colTag);
    std::string GetRecordString(const Guid &self, const std::string &recordName, const int row, const std::string &colTag);
    Guid GetRecordObject(const Guid &self, const std::string &recordName, const int row, const std::string &colTag);
    Vector2 GetRecordVector2(const Guid &self, const std::string &recordName, const int row, const std::string &colTag);
    Vector3 GetRecordVector3(const Guid &self, const std::string &recordName, const int row, const std::string &colTag);

    INT64 GetNowTime();
    Guid CreateID();
    INT64 AppID();
    INT64 AppType();
    string AppName();
    INT64 Area();

    const std::string GetScriptPath();
    void SetScriptPath(const std::string &path);

    // FOR ELEMENT MODULE
    bool ExistElementObject(const std::string &configName);
    LuaIntf::LuaRef GetConfigIDList(const string& className);
    LuaIntf::LuaRef GetConfig(const string& className);
    LuaIntf::LuaRef GetConfigByID(const string& id);
    INT64 GetElePropertyInt(const std::string &configName, const std::string &propertyName);
    double GetElePropertyFloat(const std::string &configName, const std::string &propertyName);
    std::string GetElePropertyString(const std::string &configName, const std::string &propertyName);
    Vector2 GetElePropertyVector2(const std::string &configName, const std::string &propertyName);
    Vector3 GetElePropertyVector3(const std::string &configName, const std::string &propertyName);

    // FOR NET MODULE
    // as server
    void RemoveCallBackAsServer(const int msg_id);
    void AddMsgCallBackAsServer(const int msg_id, const LuaIntf::LuaRef &luaTable, const LuaIntf::LuaRef &luaFunc);

    // as client
    void RemoveMsgCallBackAsClient(const ServerType serverType, const int msg_id);
    void AddMsgCallBackAsClient(const ServerType serverType, const int msg_id, const LuaIntf::LuaRef &luaTable, const LuaIntf::LuaRef &luaFunc);

    bool ImportProtoFile(const std::string &fileName);
    const std::string Encode(const std::string &msgTypeName, const LuaIntf::LuaRef &luaTable);
    LuaIntf::LuaRef Decode(const std::string &msgTypeName, const std::string &data);

    void SendToServerByServerID(const int server_id, const uint16_t msg_id, const std::string &data, const std::string& guid);
    void SendToAllServerByServerType(const ServerType server_type, const uint16_t msg_id, const std::string &data, const std::string& guid);

    // for net module
    void SendByFD(const socket_t fd, const uint16_t msg_id, const std::string &data, string guid);

    // for log
    void LogInfo(const std::string &logData);
    void LogError(const std::string &logData);
    void LogWarning(const std::string &logData);
    void LogDebug(const std::string &logData);

    // hot fix
    void SetVersionCode(const std::string &logData);
    const std::string &GetVersionCode();

    // FOR CLASS MODULE
    bool AddClassCallBack(std::string &className, const LuaIntf::LuaRef &luaTable, const LuaIntf::LuaRef &luaFunc);

    

  protected:
    template <typename T> bool AddLuaFuncToMap(Map<T, Map<Guid, List<LuaCallBack>>> &funcMap, const Guid &self, T key, const LuaCallBack &callback);

    template <typename T> bool AddLuaFuncToMap(Map<T, Map<Guid, List<LuaCallBack>>> &funcMap, T key, const LuaCallBack &callback);

    int OnLuaPropertyCB(const Guid &self, const std::string &propertyName, const SquickData &oldVar, const SquickData &newVar, const INT64 reason);
    int OnLuaRecordCB(const Guid &self, const RECORD_EVENT_DATA &eventData, const SquickData &oldVar, const SquickData &newVar);
    int OnLuaHeartBeatCB(const Guid &self, const std::string &strHeartBeatName, const float time, const int count);

    int OnLuaEventCB(const Guid &self, const int eventID, const DataList &argVar);

    int OnClassEventCB(const Guid &self, const std::string &className, const CLASS_OBJECT_EVENT classEvent, const DataList &var);

    void OnScriptReload();

    void OnNetMsgCallBackAsServer(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnNetMsgCallBackAsClient(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  protected:
    bool Register();
    private:
        INT64 last_update_time_ = 0;

  protected:
    IElementModule *m_element_;
    IKernelModule *m_kernel_;
    IClassModule *m_class_;
    IEventModule *m_event_;
    IScheduleModule *m_schedule_;
    INetClientModule *m_net_client_;
    INetModule *m_net_;
    ILogModule *m_log_;
    ILuaPBModule *m_pLuaPBModule;

  protected:
    std::string strVersionCode;
    LuaIntf::LuaContext mLuaContext;

    Map<std::string, Map<Guid, List<LuaCallBack>>> mxLuaPropertyCallBackFuncMap;
    Map<std::string, Map<Guid, List<LuaCallBack>>> mxLuaRecordCallBackFuncMap;
    Map<int, Map<Guid, List<LuaCallBack>>> mxLuaEventCallBackFuncMap;
    Map<std::string, Map<Guid, List<LuaCallBack>>> mxLuaHeartBeatCallBackFuncMap;
    Map<std::string, List<LuaCallBack>> mxClassEventFuncMap;
    Map<int, List<LuaCallBack>> mxNetMsgCallBackFuncMapAsServer;
    Map<int, List<LuaCallBack>> mxNetMsgCallBackFuncMapAsClient;

    std::string scriptPath = "";
    int hotFixNotifyFd = -1;
    char hotFixInotifyEventBuf[512];
};