

#include "world_module.h"
#include "plugin.h"
#include <squick/core/data_list.h>
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/struct/struct.h>

bool DBToWorldModule::Start() { return true; }

bool DBToWorldModule::Destory() { return true; }

bool DBToWorldModule::AfterStart() { return true; }

bool DBToWorldModule::Update() {
    // SQUICK_WILL_DO
    // ServerReport();
    return true;
}

void DBToWorldModule::InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) { printf("Net || umsg_id=%d\n", msg_id); }

void DBToWorldModule::OnClientDisconnect(const socket_t sock) {}

void DBToWorldModule::OnClientConnected(const socket_t sock) {}

bool DBToWorldModule::BeforeDestory() { return true; }
