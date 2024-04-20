#pragma once

#include "dyn_lib.h"
#include "i_module.h"
#include "i_plugin_manager.h"
#include <map>
#include <string>
#include <thread>
#include <time.h>

class PluginManager : public IPluginManager {
  public:
    PluginManager();
    virtual ~PluginManager();

    virtual bool LoadPluginConfig() override;

    virtual bool LoadPlugin() override;

    virtual bool Awake() override;

    virtual bool Start() override;

    virtual bool AfterStart() override;

    virtual bool CheckConfig() override;

    virtual bool ReadyUpdate() override;

    virtual bool BeforeDestroy() override;

    virtual bool Destroy() override;

    virtual bool Finalize() override;

    //////////////////////////////////////////////////////////////////////////

    virtual void Registered(IPlugin *pPlugin) override;

    virtual void UnRegistered(IPlugin *pPlugin) override;

    //////////////////////////////////////////////////////////////////////////

    virtual bool ReLoadPlugin(const std::string &pluginDLLName) override;

    virtual IPlugin *FindPlugin(const std::string &pluginName) override;

    virtual void AddModule(const std::string &moduleName, IModule *pModule) override;

    virtual void AddTestModule(const std::string &moduleName, IModule *pModule) override;

    virtual void RemoveModule(const std::string &moduleName) override;

    virtual IModule *FindModule(const std::string &moduleName) override;

    virtual IModule *FindTestModule(const std::string &moduleName) override;

    virtual std::list<IModule *> Modules() override;
    virtual std::list<IModule *> TestModules() override;

    virtual bool Update() override;

    virtual int GetAppID() const override;
    virtual void SetAppID(const int appID) override;
    virtual void SetArea(const int id) override;
    virtual int GetArea() const override;

    virtual std::vector<std::string> GetArgs() const override;
    virtual void SetArgs(const std::vector<std::string> &args) override;

    virtual std::string FindParameterValue(const std::string &header) override;

    virtual int GetAppType() const override;
    virtual void SetAppType(const int type) override;

    virtual INT64 GetStartTime() const override;
    virtual INT64 GetNowTime() const override;

    virtual const std::string &GetWorkPath() const override;
    virtual void SetWorkPath(const std::string &strPath) override;

    virtual void SetConfigName(const std::string &fileName) override;
    virtual const std::string &GetConfigName() const override;

    virtual const std::string &GetAppName() const override;

    virtual void SetAppName(const std::string &appName) override;

    virtual IPlugin *GetCurrentPlugin() override;
    virtual IModule *GetCurrentModule() override;

    virtual void SetCurrentPlugin(IPlugin *pPlugin) override;
    virtual void SetCurrentModule(IModule *pModule) override;

    virtual int GetAppCPUCount() const override;
    virtual void SetAppCPUCount(const int count) override;

    virtual bool UsingBackThread() const override;
    virtual void SetUsingBackThread(const bool b) override;

    virtual void SetGetFileContentFunctor(GET_FILECONTENT_FUNCTOR fun) override;

    virtual bool GetFileContent(const std::string &fileName, std::string &content) override;

    virtual void AddFileReplaceContent(const std::string &fileName, const std::string &content, const std::string &newValue) override;
    virtual std::vector<ReplaceContent> GetFileReplaceContents(const std::string &fileName) override;

  protected:
    bool LoadPluginLibrary(const std::string &pluginDLLName);
    bool UnLoadPluginLibrary(const std::string &pluginDLLName);

  private:
    int appID = 0;
    int appType = 0;
    int area_ = 0;
    bool usingBackThread = false;

    INT64 mnStartTime = 0;
    INT64 mnNowTime = 0;
    INT64 mnCPUCount = 8;

    std::string configPath;
    std::string configName;
    std::string appName;

    IPlugin *currentPlugin;
    IModule *currentModule;

    std::vector<std::string> args_;

    struct PluginInfo {
        std::string path = "";
        bool is_loaded = false;
        time_t loaded_time = 0;
    };

    typedef std::map<std::string, DynLib *> PluginLibMap;
    typedef std::map<std::string, IPlugin *> PluginInstanceMap;
    typedef std::map<std::string, IModule *> ModuleInstanceMap;
    typedef std::map<std::string, IModule *> TestModuleInstanceMap;
    typedef std::vector<std::pair<std::string, IModule *>> NeedUpdateModuleVec;

    typedef void (*DLL_START_PLUGIN_FUNC)(IPluginManager *pm);
    typedef void (*DLL_STOP_PLUGIN_FUNC)(IPluginManager *pm);

    std::vector<std::string> mStaticPlugin;
    std::map<std::string, std::vector<ReplaceContent>> mReplaceContent;

    std::vector<PluginInfo> plugins_;

    PluginLibMap mPluginLibMap;
    PluginInstanceMap mPluginInstanceMap;
    ModuleInstanceMap mModuleInstanceMap;
    TestModuleInstanceMap mTestModuleInstanceMap;
    NeedUpdateModuleVec mNeedUpdateModuleVec;

    GET_FILECONTENT_FUNCTOR mGetFileContentFunctor;
};