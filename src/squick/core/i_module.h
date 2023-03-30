#pragma once

#include "data_list.h"
#include "i_plugin_manager.h"
#include "list.h"
#include "map.h"
#include "smart_enum.h"
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

    virtual bool BeforeDestory() { return true; }

    virtual bool Destory() { return true; }

    virtual bool Finalize() { return true; }

    virtual bool OnReloadPlugin() { return true; }

    virtual IPluginManager *GetPluginManager() const { return pm_; }

    std::string name;
    bool is_update_;

  protected:
    IPluginManager *pm_;
};
