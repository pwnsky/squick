#pragma once

#include <squick/plugin/kernel/i_kernel_module.h>
#include "i_master_module.h"


class MasterModule
    : public IMasterModule
{

public:
    MasterModule(IPluginManager* p)
    {
        pPluginManager = p;
    }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
protected:


private:

    IKernelModule* m_pKernelModule;
};