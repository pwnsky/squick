#pragma once

#include "i_plugin_manager.h"
#include <string>

class IModule {

  public:
    IModule() : is_update_(false), pm_(NULL) {}

    virtual ~IModule() {}

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

    virtual bool Reload(int type) { return true; }

    virtual IPluginManager *GetPluginManager() const { return pm_; }

    std::string name_;
    bool is_update_;

  protected:
    IPluginManager *pm_;
};
