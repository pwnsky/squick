
#include "master_module.h"
#include "plugin.h"
#include <squick/struct/struct.h>
namespace login::client {

bool MasterModule::Start() {
    BaseStart();
    m_server_ = pm_->FindModule<server::IServerModule>();

    return true;
}

bool MasterModule::Destory() { return true; }

bool MasterModule::AfterStart() {
    AddServer(ServerType::ST_MASTER);
    return true;
}

bool MasterModule::BeforeDestory() { return false; }

bool MasterModule::Update() {
    BaseUpdate();
    return true;
}

map<int, rpc::Server> &MasterModule::GetServers() { return servers_; }

} // namespace login::client