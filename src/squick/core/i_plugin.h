
#ifndef SQUICK_INTF_PLUGIN_H
#define SQUICK_INTF_PLUGIN_H

#include "i_module.h"
#include "i_plugin_manager.h"
#include <assert.h>
#include <iostream>

#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) PP_CAT_II(~, a##b)
#define PP_CAT_II(p, res) res
#define UNIQUE_NAME(base) PP_CAT(base, __LINE__)

#define REGISTER_MODULE(pManager, classBaseName, className)                                                                                                    \
    assert((TIsDerived<classBaseName, IModule>::Result));                                                                                                      \
    assert((TIsDerived<className, classBaseName>::Result));                                                                                                    \
    IModule *UNIQUE_NAME(pRegisterModule) = new className(pManager);                                                                                           \
    UNIQUE_NAME(pRegisterModule)->name = (#classBaseName);                                                                                                     \
    pManager->AddModule(typeid(classBaseName).name(), UNIQUE_NAME(pRegisterModule));                                                                           \
    this->AddElement(typeid(classBaseName).name(), UNIQUE_NAME(pRegisterModule));

#define UNREGISTER_MODULE(pManager, classBaseName, className)                                                                                                  \
    IModule *UNIQUE_NAME(pRegisterModule) = dynamic_cast<IModule *>(pManager->FindModule(typeid(classBaseName).name()));                                       \
    pManager->RemoveModule(typeid(classBaseName).name());                                                                                                      \
    this->RemoveElement(typeid(classBaseName).name());                                                                                                         \
    delete UNIQUE_NAME(pRegisterModule);

#define CREATE_PLUGIN(pManager, className) pManager->Registered(new className(pManager));

#define DESTROY_PLUGIN(pManager, className) pManager->UnRegistered(pManager->FindPlugin(typeid(className).name()));

class IPluginManager;

class IPlugin : public IModule {
  public:
    IPlugin() {}
    virtual ~IPlugin() {}
    virtual const int GetPluginVersion() = 0;
    virtual const std::string GetPluginName() = 0;

    virtual void Install() = 0;

    virtual void Uninstall() = 0;

    void AddElement(const std::string &name, IModule *module) { mModules[name] = module; }

    IModule *GetElement(const std::string &name) {
        auto it = mModules.find(name);
        if (it != mModules.end()) {
            return it->second;
        }

        return nullptr;
    }

    void RemoveElement(const std::string &name) {
        auto it = mModules.find(name);
        if (it != mModules.end()) {
            mModules.erase(it);
        }
    }

    virtual bool Awake() {
        for (const auto &it : mModules) {
            IModule *pModule = it.second;

            pm_->SetCurrentModule(pModule);

            bool bRet = pModule->Awake();
            if (!bRet) {
                std::cout << pModule->name << std::endl;
                assert(0);
            }
        }

        return true;
    }

    virtual bool Start() {
        for (const auto &it : mModules) {
            IModule *pModule = it.second;

            pm_->SetCurrentModule(pModule);
            bool bRet = pModule->Start();
            if (!bRet) {
                std::cout << pModule->name << std::endl;
                assert(0);
            }
        }

        return true;
    }

    virtual bool AfterStart() {
        for (const auto &it : mModules) {
            IModule *pModule = it.second;

            pm_->SetCurrentModule(pModule);
            bool bRet = pModule->AfterStart();
            if (!bRet) {
                std::cout << pModule->name << std::endl;
                assert(0);
            }
        }
        return true;
    }

    virtual bool CheckConfig() {
        for (const auto &it : mModules) {
            IModule *pModule = it.second;

            pm_->SetCurrentModule(pModule);
            pModule->CheckConfig();
        }

        return true;
    }

    virtual bool ReadyUpdate() {
        for (const auto &it : mModules) {
            IModule *pModule = it.second;

            pm_->SetCurrentModule(pModule);
            pModule->ReadyUpdate();
        }

        return true;
    }

    virtual bool Update() {
        for (const auto &it : mModules) {
            IModule *pModule = it.second;

            pm_->SetCurrentModule(pModule);
            pModule->Update();
        }

        return true;
    }

    virtual bool BeforeDestroy() {
        for (const auto &it : mModules) {
            IModule *pModule = it.second;

            pm_->SetCurrentModule(pModule);
            pModule->BeforeDestroy();
        }

        return true;
    }

    virtual bool Destroy() {
        for (const auto &it : mModules) {
            IModule *pModule = it.second;

            pm_->SetCurrentModule(pModule);
            pModule->Destroy();
        }

        return true;
    }

    virtual bool Finalize() {
        for (const auto &it : mModules) {
            IModule *pModule = it.second;

            pm_->SetCurrentModule(pModule);
            pModule->Finalize();
        }

        return true;
    }

    virtual bool OnReloadPlugin() {
        for (const auto &it : mModules) {
            IModule *pModule = it.second;

            pm_->SetCurrentModule(pModule);
            pModule->OnReloadPlugin();
        }

        return true;
    }

  private:
    std::map<std::string, IModule *> mModules;
};

#endif
