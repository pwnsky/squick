#pragma once

#include "platform.h"
#include <functional>
#include <list>
#include <vector>

class IPlugin;
class IModule;
class IPluginManager;

typedef std::function<bool(IPluginManager *p, const std::string &fileName, std::string &content)> GET_FILECONTENT_FUNCTOR;

template <typename DerivedType, typename BaseType> class TIsDerived {
  public:
    static int AnyFunction(BaseType *base) { return 1; }

    static char AnyFunction(void *t2) { return 0; }

    enum {
        Result = (sizeof(int) == sizeof(AnyFunction((DerivedType *)NULL))),
    };
};

class ReplaceContent {
  public:
    ReplaceContent(const std::string content, const std::string newValue) {
        this->content = content;
        this->newValue = newValue;
    }
    std::string content;
    std::string newValue;
};

#define FIND_MODULE(classBaseName, className) assert((TIsDerived<classBaseName, IModule>::Result));

class IPluginManager {
  public:
    IPluginManager() {}

    virtual bool LoadPluginConfig() { return true; }

    virtual bool LoadPlugin() { return true; }

    virtual bool Awake() { return true; }

    virtual bool Start() { return true; }

    virtual bool AfterStart() { return true; }

    virtual bool CheckConfig() { return true; }

    virtual bool ReadyUpdate() { return true; }

    virtual bool Update() { return true; }

    virtual bool BeforeDestroy() { return true; }

    virtual bool Destroy() { return true; }

    virtual bool Finalize() { return true; }

    virtual bool OnReloadPlugin() { return true; }

    /////////////////////

    template <typename T> T *FindModule() {
        IModule *pLogicModule = FindModule(typeid(T).name());
        if (pLogicModule) {
            if (!TIsDerived<T, IModule>::Result) {
                return NULL;
            }
            // TODO OSX上dynamic_cast返回了NULL
#if PLATFORM == PLATFORM_APPLE
            T *pT = (T *)pLogicModule;
#else
            T *pT = dynamic_cast<T *>(pLogicModule);
#endif
            assert(NULL != pT);

            return pT;
        }
        assert(NULL);
        return NULL;
    }

    template <typename T> void ReplaceModule(IModule *pModule) {
        IModule *pLogicModule = FindModule(typeid(T).name());
        if (pLogicModule) {
            RemoveModule(typeid(T).name());
        }

        AddModule(typeid(T).name(), pModule);
    };

    virtual bool ReLoadPlugin(const std::string &pluginDLLName) = 0;

    virtual void Registered(IPlugin *plugin) = 0;

    virtual void UnRegistered(IPlugin *plugin) = 0;

    virtual IPlugin *FindPlugin(const std::string &pluginName) = 0;

    virtual void AddModule(const std::string &moduleName, IModule *pModule) = 0;

    virtual void AddTestModule(const std::string &moduleName, IModule *pModule) = 0;

    virtual void RemoveModule(const std::string &moduleName) = 0;

    virtual IModule *FindModule(const std::string &moduleName) = 0;

    virtual IModule *FindTestModule(const std::string &moduleName) = 0;

    virtual std::list<IModule *> Modules() = 0;
    virtual std::list<IModule *> TestModules() = 0;

    virtual int GetAppID() const = 0;
    virtual void SetAppID(const int appID) = 0;

    virtual int GetArea() const = 0;
    virtual void SetArea(const int id) = 0;

    virtual std::vector<std::string> GetArgs() const = 0;
    virtual void SetArgs(const std::vector<std::string>& id) = 0;

    virtual std::string FindParameterValue(const std::string& header) = 0;
    
    int GetArg(const std::string& header, int default_value) {
        std::string value = FindParameterValue(header);
        int ret_value;
        if (value.empty()) {
            ret_value = default_value;
        } else {
            ret_value = atoi(value.c_str());
        }
        return ret_value;
    }

    std::string GetArg(const std::string& header, const std::string &default_value) {
        std::string value = FindParameterValue(header);
        std::string ret_value;
        if (value.empty()) {
            ret_value = default_value;
        } else {
            ret_value = value;
        }
        return ret_value;
    }

    virtual int GetAppType() const = 0;
    virtual void SetAppType(const int type) = 0;

    virtual INT64 GetStartTime() const = 0;
    virtual INT64 GetNowTime() const = 0;

    virtual const std::string &GetWorkPath() const = 0;
    virtual void SetWorkPath(const std::string &strPath) = 0;

    virtual void SetConfigName(const std::string &fileName) = 0;
    virtual const std::string &GetConfigName() const = 0;

    virtual const std::string &GetAppName() const = 0;
    virtual void SetAppName(const std::string &appName) = 0;

    virtual IPlugin *GetCurrentPlugin() = 0;
    virtual IModule *GetCurrentModule() = 0;

    virtual void SetCurrentPlugin(IPlugin *pPlugin) = 0;
    virtual void SetCurrentModule(IModule *pModule) = 0;

    virtual int GetAppCPUCount() const = 0;
    virtual void SetAppCPUCount(const int count) = 0;

    virtual bool UsingBackThread() const = 0;
    virtual void SetUsingBackThread(const bool b) = 0;

    virtual void SetGetFileContentFunctor(GET_FILECONTENT_FUNCTOR fun) = 0;
    virtual bool GetFileContent(const std::string &fileName, std::string &content) = 0;

    virtual void AddFileReplaceContent(const std::string &fileName, const std::string &content, const std::string &newValue) = 0;
    virtual std::vector<ReplaceContent> GetFileReplaceContents(const std::string &fileName) = 0;
};
