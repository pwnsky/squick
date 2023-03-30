#include "player_manager_module.h"
#include <server/db_proxy/logic/common_redis_module.h>

namespace game::player {
bool PlayerManagerModule::Start() {
    m_element_ = pm_->FindModule<IElementModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_net_ = pm_->FindModule<INetModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_pGameToDBModule = pm_->FindModule<IGameServerToDBModule>();
    m_log_ = pm_->FindModule<ILogModule>();

    m_pGameServerNet_ServerModule = pm_->FindModule<IGameServerNet_ServerModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_schedule_ = pm_->FindModule<IScheduleModule>();
    m_data_tail_ = pm_->FindModule<IDataTailModule>();
    m_scene_ = pm_->FindModule<ISceneModule>();

    m_event_ = pm_->FindModule<IEventModule>();
    m_room_ = pm_->FindModule<IRoomModule>();

    m_gameplay_manager_ = pm_->FindModule<play::IGameplayManagerModule>();

    return true;
}

bool PlayerManagerModule::AfterStart() {
    m_kernel_->AddClassCallBack(excel::Player::ThisName(), this, &PlayerManagerModule::OnPlayerObjectEvent);
    return true;
}

bool PlayerManagerModule::ReadyUpdate() {
    m_net_->AddReceiveCallBack(SquickStruct::GameLobbyRPC::REQ_ENTER, this, &PlayerManagerModule::OnReqPlayerEnter);
    m_net_->AddReceiveCallBack(SquickStruct::GameLobbyRPC::REQ_LEAVE, this, &PlayerManagerModule::OnReqPlayerLeave);
    m_net_client_->AddReceiveCallBack(ServerType::ST_DB_PROXY, SquickStruct::DbProxyRPC::ACK_PLAYER_DATA_LOAD, this,
                                           &PlayerManagerModule::OnAckPlayerDataLoad);

    return true;
}

void PlayerManagerModule::OnReqPlayerEnter(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid clientID;
    SquickStruct::ReqEnter xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, clientID)) {
        return;
    }
    dout << "请求进入大厅 " << clientID.ToString() << std::endl;

    std::shared_ptr<IGameServerNet_ServerModule::ProxyServerInfo> pGateServerinfo = m_pGameServerNet_ServerModule->GetProxyServerInfoBySockIndex(sock);
    if (nullptr == pGateServerinfo) {
        return;
    }

    int gateID = -1;
    if (pGateServerinfo->xServerData.pData) {
        gateID = pGateServerinfo->xServerData.pData->server_id();
    }

    if (gateID < 0) {
        return;
    }

    // 暂时用
    if (!m_pGameServerNet_ServerModule->AddPlayerProxyInfo(clientID, clientID, gateID)) {
        return;
    }

    m_net_client_->SendBySuitWithOutHead(ServerType::ST_DB_PROXY, sock, SquickStruct::DbProxyRPC::REQ_PLAYER_DATA_LOAD,
                                              std::string(msg, len));
}

// 返回角色数据
void PlayerManagerModule::OnAckPlayerDataLoad(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    //dout << "返回角色数据\n";
    Guid clientID;
    SquickStruct::PlayerData xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, clientID)) {
        return;
    }

    dout << "返回角色数据 clientID: " << clientID.ToString() << std::endl;
    // 告诉客户端进入游戏成功
    // 暂时用，之后将绑定为ObjectID
    // Guid objectID = INetModule::ProtobufToStruct(xMsg.object());
    Guid objectID = clientID;

    mxObjectDataCache[objectID] = xMsg; // 缓存玩家数据

    Player *player = nullptr;
    // 查找缓存中不存在玩家重新生成玩家数据
    if (m_players.find(objectID) == m_players.end()) {
        player = new Player();
        m_players[objectID] = player;
    }

    player = m_players[objectID];
    // 获取到数据后，再创建玩家对象
    player->OnEnterGame();
    player->loginTime = time(nullptr);


    /*
    if (m_kernel_->GetObject(objectID)) { // 存在玩家，销毁对象
        // it should be rebind with proxy's netobject
        m_kernel_->DestroyObject(objectID);
    }

    DataList var;
    std::shared_ptr<IGameServerNet_ServerModule::GateBaseInfo>  pGateInfo = m_pGameServerNet_ServerModule->GetPlayerGateInfo(clientID);
    if (nullptr == pGateInfo)
    {
            dout << "Error to load GateBaseInfo: ClientID: " << clientID.ToString() << std::endl;
            return;
    }

    var.AddString(SquickProtocol::Player::GateID());
    var.AddInt(pGateInfo->gateID);

    var.AddString(SquickProtocol::Player::GameID());
    var.AddInt(pm_->GetAppID());

    var.AddString(SquickProtocol::Player::Connection());
    var.AddInt(1);

    std::shared_ptr<IObject> pObject = m_kernel_->CreateObject(objectID, 1, 0, SquickProtocol::Player::ThisName(), "", var);
    */

    SquickStruct::AckEnter ack;
    ack.set_code(0);
    *ack.mutable_client() = INetModule::StructToProtobuf(clientID);
    *ack.mutable_object() = INetModule::StructToProtobuf(objectID);

    m_pGameServerNet_ServerModule->SendMsgPBToProxy(SquickStruct::ACK_ENTER, ack, clientID);
}

// 玩家对象事件
int PlayerManagerModule::OnPlayerObjectEvent(const Guid &self, const std::string &className, const CLASS_OBJECT_EVENT classEvent, const DataList &var) {
    // 离线
    if (CLASS_OBJECT_EVENT::COE_DESTROY == classEvent) {
        // m_data_tail_->LogObjectData(self);
        //  玩家离线
        dout << "玩家数据对象销毁: " << self.ToString() << " \n";
        m_kernel_->SetPropertyInt(self, excel::Player::LastOfflineTime(), SquickGetTimeS());
        SaveDataToDb(self); // 保存数据到数据库

        Player *player = m_players[self];
        if (player == nullptr) {
            dout << "Player offline not found!\n";
            return -1;
        }

        player->OnOffline();
        player->offlineTime = time(nullptr); // 记录离线时间，由Player Manager定时清理已离线的玩家数据
        m_offlineCachePlayers[self] = player;
    } else if (CLASS_OBJECT_EVENT::COE_CREATE_LOADDATA == classEvent) {
        dout << "玩家数据对象加载\n";
        // m_data_tail_->StartTrail(self);
        // m_data_tail_->LogObjectData(self);

        LoadDataFromDb(self);
        m_kernel_->SetPropertyInt(self, excel::Player::OnlineTime(), SquickGetTimeS());
    } else if (CLASS_OBJECT_EVENT::COE_CREATE_FINISH == classEvent) {
        dout << "玩家加载数据完成\n";
        auto it = mxObjectDataCache.find(self);
        if (it != mxObjectDataCache.end()) {
            mxObjectDataCache.erase(it);
        }

        // 每3分钟 保存一次玩家数据到数据库
        // m_schedule_->AddSchedule(self, "SaveDataOnTime", this, &PlayerManagerModule::SaveDataOnTime, 180.0f, -1);
    }
    return 0;
}

// 从数据库读取玩家数据
void PlayerManagerModule::LoadDataFromDb(const Guid &self) {
    auto it = mxObjectDataCache.find(self);
    if (it != mxObjectDataCache.end()) {
        std::shared_ptr<IObject> xObject = m_kernel_->GetObject(self);
        if (xObject) {
            std::shared_ptr<IPropertyManager> xPropManager = xObject->GetPropertyManager();
            std::shared_ptr<IRecordManager> xRecordManager = xObject->GetRecordManager();

            if (xPropManager) {
                CommonRedisModule::ConvertPBToPropertyManager(it->second.property(), xPropManager);
            }

            if (xRecordManager) {
                CommonRedisModule::ConvertPBToRecordManager(it->second.record(), xRecordManager);
            }
            mxObjectDataCache.erase(it);
            xObject->SetPropertyInt(excel::Player::GateID(), pm_->GetAppID());
            auto playerGateInfo = m_pGameServerNet_ServerModule->GetPlayerProxyInfo(self);
            if (playerGateInfo) {
                xObject->SetPropertyInt(excel::Player::GateID(), playerGateInfo->proxy_id_);
            }
        }
    }
}

// 保存玩家数据到数据库
void PlayerManagerModule::SaveDataToDb(const Guid &self) {
    std::shared_ptr<IObject> xObject = m_kernel_->GetObject(self);
    if (xObject) {
        std::shared_ptr<IPropertyManager> xPropManager = xObject->GetPropertyManager();
        std::shared_ptr<IRecordManager> xRecordManager = xObject->GetRecordManager();
        SquickStruct::PlayerData xDataPack;

        *xDataPack.mutable_object() = INetModule::StructToProtobuf(self);

        *(xDataPack.mutable_property()->mutable_player_id()) = INetModule::StructToProtobuf(self);
        *(xDataPack.mutable_record()->mutable_player_id()) = INetModule::StructToProtobuf(self);

        if (xPropManager) {
            CommonRedisModule::ConvertPropertyManagerToPB(xPropManager, xDataPack.mutable_property(), false, true);
        }

        if (xRecordManager) {
            CommonRedisModule::ConvertRecordManagerToPB(xRecordManager, xDataPack.mutable_record(), false, true);
        }
        m_net_client_->SendSuitByPB(ServerType::ST_DB_PROXY, self.GetData(), SquickStruct::DbProxyRPC::REQ_PLAYER_DATA_SAVE, xDataPack);
    }
}

// 定时保存玩家数据
int PlayerManagerModule::SaveDataOnTime(const Guid &self, const std::string &name, const float fIntervalTime, const int count) {
    SaveDataToDb(self);
    return 0;
}

bool PlayerManagerModule::Destory() { return true; }

bool PlayerManagerModule::Update() {
    dout << "Update...\n";
    return true;
}

// 发送数据给客户端，用于给player.cc使用
void PlayerManagerModule::OnSendToClient(const uint16_t msg_id, google::protobuf::Message &xMsg, const Guid &client_id) {
    m_pGameServerNet_ServerModule->SendMsgPBToProxy(msg_id, xMsg, client_id);
}

// virtual  ();
// 发送数据给客户端，用于给player.cc使用
Player *PlayerManagerModule::GetPlayer(const Guid &clientID) { return m_players[clientID]; }

int PlayerManagerModule::GetPlayerRoomID(const Guid &clientID) {
    auto player = m_players[clientID];
    if (player != nullptr) {
        return player->GetRoomID();
    }
    return -1;
}

void PlayerManagerModule::SetPlayerRoomID(const Guid &clientID, int groupID) {
    auto player = m_players[clientID];
    if (player != nullptr) {
        player->SetRoomID(groupID);
    }
}

int PlayerManagerModule::GetPlayerGameplayID(const Guid &clientID) {
    auto player = m_players[clientID];
    if (player != nullptr) {
        return player->GetGameplayID();
    }
    return -1;
}

void PlayerManagerModule::SetPlayerGameplayID(const Guid &clientID, int groupID) {
    auto player = m_players[clientID];
    if (player != nullptr) {
        player->SetGameplayID(groupID);
    }
}

void PlayerManagerModule::OnReqPlayerLeave(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ReqLeave xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }
    dout << "玩家离线: " << nPlayerID.ToString() << std::endl;

    if (nPlayerID.IsNull()) {
        return;
    }

    // 先退出gameplay
    m_gameplay_manager_->GameplayPlayerQuit(nPlayerID);

    // 退出room
    m_room_->RoomQuit(nPlayerID);

    // 离线退出 Gameplay

    // m_kernel_->SetPropertyInt(nPlayerID, SquickProtocol::IObject::Connection(), 0);
    // m_kernel_->DestroyObject(nPlayerID);
    // m_pGameServerNet_ServerModule->RemovePlayerGateInfo(nPlayerID);
}

} // namespace game::player