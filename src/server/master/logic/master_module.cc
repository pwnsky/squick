
#include "master_module.h"
#include "plugin.h"

bool MasterModule::Start() {

    m_pKernelModule = pPluginManager->FindModule<IKernelModule>();

    return true;
}

bool MasterModule::Destory() { return true; }

bool MasterModule::Update() { return true; }
