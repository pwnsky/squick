#include "plugin_manager.h"
#include "i_plugin.h"
#include "platform.h"
#include "third_party/rapidxml/rapidxml.hpp"
#include "third_party/rapidxml/rapidxml_iterators.hpp"
#include "third_party/rapidxml/rapidxml_print.hpp"
#include "third_party/rapidxml/rapidxml_utils.hpp"

PluginManager::PluginManager() : IPluginManager() {
    appID = 0;
    mbIsDocker = false;
    area_ = 0;

    currentPlugin = nullptr;
    currentModule = nullptr;

#ifdef SQUICK_DYNAMIC_PLUGIN
    mbStaticPlugin = false;
#else
    mbStaticPlugin = true;
#endif

    mnStartTime = time(NULL);
    mnNowTime = mnStartTime;

    mGetFileContentFunctor = nullptr;

    configPath = "..";                       // 主要服务路径
    configName = "config/plugin/plugin.xml"; // 默认插件加载路径
}

PluginManager::~PluginManager() {}

bool PluginManager::LoadPlugin() {
#ifdef DEBUG
    std::cout << "---- LoadPlugin ----" << std::endl;
#endif
    PluginNameMap::iterator it = mPluginNameMap.begin();
    for (; it != mPluginNameMap.end(); ++it) {
#ifdef DEBUG
        std::cout << "---- DYNAMIC Plugin: " << it->first << std::endl;
#endif
        LoadPluginLibrary(it->first);
    }

    return true;
}

bool PluginManager::Awake() {
#ifdef DEBUG
    std::cout << "----Awake----" << std::endl;
#endif
    PluginInstanceMap::iterator itAfterInstance = mPluginInstanceMap.begin();
    for (; itAfterInstance != mPluginInstanceMap.end(); itAfterInstance++) {
        SetCurrentPlugin(itAfterInstance->second);
        itAfterInstance->second->Awake();
    }

    return true;
}

inline bool PluginManager::Start() {
#ifdef DEBUG
    std::cout << "----Start----" << std::endl;
#endif
    PluginInstanceMap::iterator itInstance = mPluginInstanceMap.begin();
    for (; itInstance != mPluginInstanceMap.end(); itInstance++) {
        SetCurrentPlugin(itInstance->second);
#ifdef DEBUG
        // std::cout << "Loaded Plugin: "  << itInstance->first << std::endl;
#endif
        itInstance->second->Start();
    }

    return true;
}

// 加载插件配置
bool PluginManager::LoadPluginConfig() {
    std::string content;
    std::string strFilePath = GetConfigPath() + "/" + configName;
    GetFileContent(strFilePath, content);

    rapidxml::xml_document<> xDoc;
    xDoc.parse<0>((char *)content.c_str());

    rapidxml::xml_node<> *pRoot = xDoc.first_node();
    if (pRoot == nullptr) {
        std::cout << "Cannot load plugin config from :" << strFilePath << std::endl;
        std::cout << "Please check your working directory, make sure config file in the [../config/plugin/] directory.\n";
        return false;
    }
    rapidxml::xml_node<> *pAppNameNode = pRoot->first_node(appName.c_str());
    if (pAppNameNode) {
        for (rapidxml::xml_node<> *pPluginNode = pAppNameNode->first_node("Plugin"); pPluginNode; pPluginNode = pPluginNode->next_sibling("Plugin")) {
            const char *pluginName = pPluginNode->first_attribute("Name")->value();

            mPluginNameMap.insert(PluginNameMap::value_type(pluginName, true));

            // std::cout << pluginName << std::endl;
        }
    } else {

        for (rapidxml::xml_node<> *pServerNode = pRoot->first_node(); pServerNode; pServerNode = pServerNode->next_sibling()) {

            for (rapidxml::xml_node<> *pPluginNode = pServerNode->first_node("Plugin"); pPluginNode; pPluginNode = pPluginNode->next_sibling("Plugin")) {
                const char *pluginName = pPluginNode->first_attribute("Name")->value();
                if (mPluginNameMap.find(pluginName) == mPluginNameMap.end()) {
                    mPluginNameMap.insert(PluginNameMap::value_type(pluginName, true));
                }
            }
        }
    }

    return true;
}

void PluginManager::Registered(IPlugin *plugin) {
    const std::string &pluginName = plugin->GetPluginName();
    if (!FindPlugin(pluginName)) {
        mPluginInstanceMap.insert(PluginInstanceMap::value_type(pluginName, plugin));
        plugin->Install();
    } else {
#ifdef DEBUG
        std::cout << "Registered Plugin:    " << pluginName << std::endl;
#endif
        assert(0);
    }
}

void PluginManager::UnRegistered(IPlugin *plugin) {
#ifdef DEBUG
    std::cout << "UnRegistered Plugin:    " << plugin->GetPluginName() << std::endl;
#endif
    PluginInstanceMap::iterator it = mPluginInstanceMap.find(plugin->GetPluginName());
    if (it != mPluginInstanceMap.end()) {
        it->second->Uninstall();
        delete it->second;
        it->second = NULL;
        mPluginInstanceMap.erase(it);
    }
}

bool PluginManager::ReLoadPlugin(const std::string &pluginDLLName) {
    // 1.shut all module of this plugin
    // 2.unload this plugin
    // 3.load new plugin
    // 4.init new module
    // 5.tell others who has been reloaded
    PluginInstanceMap::iterator itInstance = mPluginInstanceMap.find(pluginDLLName);
    if (itInstance == mPluginInstanceMap.end()) {
        return false;
    }
    // 1
    IPlugin *pPlugin = itInstance->second;
    pPlugin->BeforeDestory();
    pPlugin->Destory();
    pPlugin->Finalize();

    // 2
    PluginLibMap::iterator it = mPluginLibMap.find(pluginDLLName);
    if (it != mPluginLibMap.end()) {
        DynLib *pLib = it->second;
        DLL_STOP_PLUGIN_FUNC pFunc = (DLL_STOP_PLUGIN_FUNC)pLib->GetSymbol("SquickPluginUnload");

        if (pFunc) {
            pFunc(this);
        }

        pLib->UnLoad();

        delete pLib;
        pLib = NULL;
        mPluginLibMap.erase(it);
    }

    // 3
    DynLib *pLib = new DynLib(pluginDLLName);
    bool bLoad = pLib->Load();
    if (bLoad) {
        mPluginLibMap.insert(PluginLibMap::value_type(pluginDLLName, pLib));

        DLL_START_PLUGIN_FUNC pFunc = (DLL_START_PLUGIN_FUNC)pLib->GetSymbol("SquickPluginLoad");
        if (!pFunc) {
            std::cout << "Reload Find function SquickPluginLoad Failed in [" << pLib->GetName() << "]" << std::endl;
            assert(0);
            return false;
        }

        pFunc(this);
    } else {
#if PLATFORM == PLATFORM_LINUX
        char *error = dlerror();
        if (error) {
            std::cout << stderr << " Reload shared lib[" << pLib->GetName() << "] failed, ErrorNo. = [" << error << "]" << std::endl;
            std::cout << "Reload [" << pLib->GetName() << "] failed" << std::endl;
            assert(0);
            return false;
        }
#elif PLATFORM == PLATFORM_WIN
        std::cout << stderr << " Reload DLL[" << pLib->GetName() << "] failed, ErrorNo. = [" << GetLastError() << "]" << std::endl;
        std::cout << "Reload [" << pLib->GetName() << "] failed" << std::endl;
        assert(0);
        return false;
#endif
    }

    // 4
    PluginInstanceMap::iterator itReloadInstance = mPluginInstanceMap.begin();
    for (; itReloadInstance != mPluginInstanceMap.end(); itReloadInstance++) {
        if (pluginDLLName != itReloadInstance->first) {
            itReloadInstance->second->OnReloadPlugin();
        }
    }
    return true;
}

IPlugin *PluginManager::FindPlugin(const std::string &pluginName) {
    PluginInstanceMap::iterator it = mPluginInstanceMap.find(pluginName);
    if (it != mPluginInstanceMap.end()) {
        return it->second;
    }

    return NULL;
}

bool PluginManager::Update() {
    mnNowTime = time(NULL);

    bool bRet = true;

    for (auto &xPair : mNeedUpdateModuleVec) {
        bool tembRet = xPair.second->Update();
        bRet = bRet && tembRet;
    }
    // sleep(1);

    return bRet;
}

inline int PluginManager::GetAppID() const { return appID; }

inline void PluginManager::SetAppID(const int id) { appID = id; }

inline void PluginManager::SetArea(const int id) { area_ = id; }

inline int PluginManager::GetArea() const { return area_; }

inline std::vector<std::string> PluginManager::GetArgs() const { return args_; }

inline void PluginManager::SetArgs(const std::vector<std::string> & args) { args_ = args; }

inline std::string PluginManager::FindParameterValue(const std::string& header) {
    for (int i = 0; i < args_.size(); i++) {
        std::string name = args_[i];
        if (name.find(header) != string::npos) {
            name.erase(0, header.length());
            return name;
        }
    }

    return "";
}

bool PluginManager::IsRunningDocker() const { return mbIsDocker; }

void PluginManager::SetRunningDocker(bool bDocker) { mbIsDocker = bDocker; }

inline INT64 PluginManager::GetStartTime() const { return mnStartTime; }

inline INT64 PluginManager::GetNowTime() const { return mnNowTime; }

inline const std::string &PluginManager::GetConfigPath() const { return configPath; }

inline void PluginManager::SetConfigPath(const std::string &strPath) { configPath = strPath; }

void PluginManager::SetConfigName(const std::string &fileName) {
    if (fileName.empty()) {
        return;
    }

    if (fileName.find(".xml") == string::npos) {
        return;
    }
    configName = "config/plugin/" + fileName;
}

const std::string &PluginManager::GetConfigName() const { return configName; }

const std::string &PluginManager::GetAppName() const { return appName; }

void PluginManager::SetAppName(const std::string &name) {
    if (name.empty()) {
        return;
    }
    appName = name;
}

const std::string &PluginManager::GetLogConfigName() const { return logConfigName; }

void PluginManager::SetLogConfigName(const std::string &name) { logConfigName = name; }

IPlugin *PluginManager::GetCurrentPlugin() { return currentPlugin; }

IModule *PluginManager::GetCurrentModule() { return currentModule; }

void PluginManager::SetCurrentPlugin(IPlugin *pPlugin) { currentPlugin = pPlugin; }

void PluginManager::SetCurrentModule(IModule *pModule) { currentModule = pModule; }

void PluginManager::SetGetFileContentFunctor(GET_FILECONTENT_FUNCTOR fun) { mGetFileContentFunctor = fun; }

bool PluginManager::GetFileContent(const std::string &fileName, std::string &content) {
    if (mGetFileContentFunctor) {
        return mGetFileContentFunctor(this, fileName, content);
    }

    FILE *fp = fopen(fileName.c_str(), "rb");
    if (!fp) {
        return false;
    }

    fseek(fp, 0, SEEK_END);
    const long filelength = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    content.resize(filelength);
    fread((void *)content.data(), filelength, 1, fp);
    fclose(fp);

    return true;
}

void PluginManager::AddModule(const std::string &moduleName, IModule *pModule) {
#ifdef DEBUG
    std::cout << "Add Module: " << moduleName << std::endl;
#endif
    if (!FindModule(moduleName)) {
        mModuleInstanceMap.insert(ModuleInstanceMap::value_type(moduleName, pModule));

        if (pModule->is_update_) {
            mNeedUpdateModuleVec.push_back(std::make_pair(moduleName, pModule));
        }
    }
}

void PluginManager::AddTestModule(const std::string &moduleName, IModule *pModule) {
#ifdef DEBUG
    std::cout << "Add Test Module: " << moduleName << std::endl;
#endif
    if (!FindTestModule(moduleName)) {
        mTestModuleInstanceMap.insert(TestModuleInstanceMap::value_type(moduleName, pModule));
    }
}

void PluginManager::RemoveModule(const std::string &moduleName) {

#ifdef DEBUG
    std::cout << "Remove Module: " << moduleName << std::endl;
#endif

    ModuleInstanceMap::iterator it = mModuleInstanceMap.find(moduleName);
    if (it != mModuleInstanceMap.end()) {
        mModuleInstanceMap.erase(it);

        auto iter = std::find_if(mNeedUpdateModuleVec.begin(), mNeedUpdateModuleVec.end(),
                                 [&moduleName](const std::pair<std::string, IModule *> &xPair) -> bool { return xPair.first == moduleName; });

        if (iter != mNeedUpdateModuleVec.end())
            mNeedUpdateModuleVec.erase(iter);
    }
}

IModule *PluginManager::FindModule(const std::string &moduleName) {
    if (moduleName.empty()) {
        return nullptr;
    }

    ModuleInstanceMap::iterator it = mModuleInstanceMap.find(moduleName);
    if (it != mModuleInstanceMap.end()) {
        return it->second; // 找到
    }

    if (this->GetCurrentModule()) {
        std::cout << this->GetCurrentModule()->name << " want to find module: " << moduleName << " but got null_ptr!!!" << std::endl;
    }

    return nullptr;
}

IModule *PluginManager::FindTestModule(const std::string &moduleName) {
    if (moduleName.empty()) {
        return NULL;
    }

    TestModuleInstanceMap::iterator it = mTestModuleInstanceMap.find(moduleName);
    if (it != mTestModuleInstanceMap.end()) {
        return it->second;
    }

    return NULL;
}

std::list<IModule *> PluginManager::Modules() {
    std::list<IModule *> xModules;

    ModuleInstanceMap::iterator itCheckInstance = mModuleInstanceMap.begin();
    for (; itCheckInstance != mModuleInstanceMap.end(); itCheckInstance++) {
        xModules.push_back(itCheckInstance->second);
    }

    return xModules;
}

bool PluginManager::AfterStart() {
#ifdef DEBUG
    std::cout << "----AfterStart----" << std::endl;
#endif
    PluginInstanceMap::iterator itAfterInstance = mPluginInstanceMap.begin();
    for (; itAfterInstance != mPluginInstanceMap.end(); itAfterInstance++) {
        SetCurrentPlugin(itAfterInstance->second);
        itAfterInstance->second->AfterStart();
    }

    return true;
}

bool PluginManager::CheckConfig() {
#ifdef DEBUG
    std::cout << "----CheckConfig----" << std::endl;
#endif
    PluginInstanceMap::iterator itCheckInstance = mPluginInstanceMap.begin();
    for (; itCheckInstance != mPluginInstanceMap.end(); itCheckInstance++) {
        SetCurrentPlugin(itCheckInstance->second);
        itCheckInstance->second->CheckConfig();
    }

    return true;
}

bool PluginManager::ReadyUpdate() {
#ifdef DEBUG
    std::cout << "----ReadyUpdate----" << std::endl;
#endif
    PluginInstanceMap::iterator itCheckInstance = mPluginInstanceMap.begin();
    for (; itCheckInstance != mPluginInstanceMap.end(); itCheckInstance++) {
        SetCurrentPlugin(itCheckInstance->second);
        itCheckInstance->second->ReadyUpdate();
    }

    return true;
}

bool PluginManager::BeforeDestory() {
    PluginInstanceMap::iterator itBeforeInstance = mPluginInstanceMap.begin();
    for (; itBeforeInstance != mPluginInstanceMap.end(); itBeforeInstance++) {
        SetCurrentPlugin(itBeforeInstance->second);
        itBeforeInstance->second->BeforeDestory();
    }

    return true;
}

bool PluginManager::Destory() {
    PluginInstanceMap::iterator itInstance = mPluginInstanceMap.begin();
    for (; itInstance != mPluginInstanceMap.end(); ++itInstance) {
        SetCurrentPlugin(itInstance->second);
        itInstance->second->Destory();
    }

    return true;
}

bool PluginManager::Finalize() {
    PluginInstanceMap::iterator itInstance = mPluginInstanceMap.begin();
    for (; itInstance != mPluginInstanceMap.end(); itInstance++) {
        SetCurrentPlugin(itInstance->second);
        itInstance->second->Finalize();
    }

    ////////////////////////////////////////////////

    PluginNameMap::iterator it = mPluginNameMap.begin();
    for (; it != mPluginNameMap.end(); it++) {
        UnLoadPluginLibrary(it->first);
    }

    mPluginInstanceMap.clear();
    mPluginNameMap.clear();

    return true;
}

bool PluginManager::LoadPluginLibrary(const std::string &pluginDLLName) {
    PluginLibMap::iterator it = mPluginLibMap.find(pluginDLLName);
    if (it == mPluginLibMap.end()) {
        DynLib *pLib = new DynLib(pluginDLLName);
        bool bLoad = pLib->Load();

        if (bLoad) {
            mPluginLibMap.insert(PluginLibMap::value_type(pluginDLLName, pLib));

            DLL_START_PLUGIN_FUNC pFunc = (DLL_START_PLUGIN_FUNC)pLib->GetSymbol("SquickPluginLoad");
            if (!pFunc) {
                std::cout << "Find function SquickPluginLoad Failed in [" << pLib->GetName() << "]" << std::endl;
                assert(0);
                return false;
            }

            pFunc(this);

            return true;
        } else {

#if PLATFORM == PLATFORM_LINUX || PLATFORM == PLATFORM_APPLE
            char *error = dlerror();
            if (error) {
                std::cout << stderr << " Load shared lib[" << pLib->GetName() << "] failed, ErrorNo. = [" << error << "]" << std::endl;
                std::cout << "Load [" << pLib->GetName() << "] failed" << std::endl;
                assert(0);
                return false;
            }
#elif PLATFORM == PLATFORM_WIN
            std::cout << stderr << " Load DLL[" << pLib->GetName() << "] failed, ErrorNo. = [" << GetLastError() << "]" << std::endl;
            std::cout << "Load [" << pLib->GetName() << "] failed" << std::endl;
            assert(0);
            return false;
#endif // PLATFORM
        }
    }

    return false;
}

bool PluginManager::UnLoadPluginLibrary(const std::string &pluginDLLName) {
    PluginLibMap::iterator it = mPluginLibMap.find(pluginDLLName);
    if (it != mPluginLibMap.end()) {
        DynLib *pLib = it->second;
        DLL_STOP_PLUGIN_FUNC pFunc = (DLL_STOP_PLUGIN_FUNC)pLib->GetSymbol("SquickPluginUnload");

        if (pFunc) {
            pFunc(this);
        }

        pLib->UnLoad();

        delete pLib;
        pLib = NULL;
        mPluginLibMap.erase(it);

        return true;
    }

    return false;
}

void PluginManager::AddFileReplaceContent(const std::string &fileName, const std::string &content, const std::string &newValue) {
    auto it = mReplaceContent.find(fileName);
    if (it == mReplaceContent.end()) {
        std::vector<ReplaceContent> v;
        v.push_back(ReplaceContent(content, newValue));

        mReplaceContent.insert(std::make_pair(fileName, v));
    } else {
        it->second.push_back(ReplaceContent(content, newValue));
    }
}

std::vector<ReplaceContent> PluginManager::GetFileReplaceContents(const std::string &fileName) {
    auto it = mReplaceContent.find(fileName);
    if (it != mReplaceContent.end()) {
        return it->second;
    }

    return std::vector<ReplaceContent>();
}

std::list<IModule *> PluginManager::TestModules() {
    std::list<IModule *> xModules;

    TestModuleInstanceMap::iterator itCheckInstance = mTestModuleInstanceMap.begin();
    for (; itCheckInstance != mTestModuleInstanceMap.end(); itCheckInstance++) {
        xModules.push_back(itCheckInstance->second);
    }

    return xModules;
}

int PluginManager::GetAppCPUCount() const { return (int)mnCPUCount; }

void PluginManager::SetAppCPUCount(const int count) { mnCPUCount = count; }

int PluginManager::GetAppType() const { return appType; }

void PluginManager::SetAppType(const int type) { appType = type; }

bool PluginManager::UsingBackThread() const { return usingBackThread; }

void PluginManager::SetUsingBackThread(const bool b) { usingBackThread = b; }
