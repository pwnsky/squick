#pragma once

#include <squick/core/base.h>
namespace tutorial {
// You can use this class export your function, Other module can call your module's function
class ISimpleModule : public IModule {
public:
    virtual int MyExportFunc() = 0;
};

class SimpleModule : public ISimpleModule {
public:
    SimpleModule(IPluginManager *p) {
        is_update_ = true; // If is_update_ = false, This module's Update() function not be called per frame.
        pm_ = p; // Plugin manager base class, you can use it get somewhat you want.
    }

    virtual bool Start();
    virtual bool AfterStart();
    virtual bool Update();
private:
};



}