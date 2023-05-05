#include "player_manager_module.h"

namespace lobby::player {
bool PlayerManagerModule::Start() {
    m_element_ = pm_->FindModule<IElementModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_net_ = pm_->FindModule<INetModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_log_ = pm_->FindModule<ILogModule>();

    m_node_ = pm_->FindModule<node::INodeModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_schedule_ = pm_->FindModule<IScheduleModule>();
    m_data_tail_ = pm_->FindModule<IDataTailModule>();

    m_event_ = pm_->FindModule<IEventModule>();

    return true;
}

bool PlayerManagerModule::AfterStart() {
    m_kernel_->AddClassCallBack(excel::Player::ThisName(), this, &PlayerManagerModule::OnPlayerObjectEvent);
    return true;
}

bool PlayerManagerModule::ReadyUpdate() {
    m_net_->AddReceiveCallBack(rpc::LobbyBaseRPC::REQ_ENTER, this, &PlayerManagerModule::OnReqPlayerEnter);
    m_net_->AddReceiveCallBack(rpc::LobbyBaseRPC::REQ_LEAVE, this, &PlayerManagerModule::OnReqPlayerLeave);
    m_net_->AddReceiveCallBack(rpc::LobbyBaseRPC::REQ_RECONNECT, this, &PlayerManagerModule::OnReqPlayerReconnect);

    m_net_client_->AddReceiveCallBack(ServerType::ST_DB_PROXY, rpc::DbProxyRPC::ACK_PLAYER_DATA_LOAD, this,
                                      &PlayerManagerModule::OnAckPlayerDataLoad); // 每一次进入游戏都从数据库拉取一下

    m_net_->AddReceiveCallBack(rpc::LobbyPlayerRPC::REQ_PLAYER_EQUIPMENT, this, &PlayerManagerModule::OnReqPlayerEquipment);
    m_net_->AddReceiveCallBack(rpc::LobbyPlayerRPC::REQ_PLAYER_DATA, this, &PlayerManagerModule::OnReqPlayerData);
    return true;
}

void PlayerManagerModule::OnReqPlayerEnter(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid guid;
    rpc::ReqEnter xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, guid)) {
        return;
    }
    dout << "请求进入大厅 " << guid.ToString() << std::endl;

    std::string oguid = guid.ToString();

    // 检查是否是重连的
    auto iter = offline_players_.find(oguid);
    if (iter != offline_players_.end()) {
        dout << "玩家: " << oguid << " 断线重连成功\n";
        offline_players_.erase(iter);
        rpc::AckEnter ack;
        ack.set_code(0);
        *ack.mutable_guid() = INetModule::StructToProtobuf(guid);
        *ack.mutable_object() = INetModule::StructToProtobuf(guid);
        m_net_->SendMsgPB(rpc::ACK_ENTER, ack, sock);
        return;
    }

    m_net_client_->SendBySuitWithOutHead(ServerType::ST_DB_PROXY, sock, rpc::DbProxyRPC::REQ_PLAYER_DATA_LOAD, std::string(msg, len));
}

// 返回角色数据
void PlayerManagerModule::OnAckPlayerDataLoad(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    // dout << "返回角色数据\n";
    Guid guid;
    rpc::DbPlayerData data;
    if (!m_net_->ReceivePB(msg_id, msg, len, data, guid)) {
        return;
    }

    dout << "返回角色数据 guid: " << guid.ToString() << std::endl;
    // 告诉客户端进入游戏成功
    // 暂时用，之后将绑定为ObjectID
    // Guid objectID = INetModule::ProtobufToStruct(xMsg.object());
    Guid objectID = guid;

    // mxObjectDataCache[objectID] = xMsg; // 缓存玩家数据

    Player *player = nullptr;
    string object_id = objectID.ToString();
    // 查找缓存中不存在玩家重新生成玩家数据
    if (players_.find(object_id) == players_.end()) {
        player = new Player();
        players_[object_id] = player;
    }

    player = players_[object_id];
    // 获取到数据后，再创建玩家对象
    player->OnEnterGame();
    player->login_time_ = time(nullptr);
    player->account_ = data.account();
    player->guid_ = INetModule::ProtobufToStruct(data.guid());
    dout << "Player info: account:" << data.account() << " guid: " << player->guid_.ToString() << std::endl;

    rpc::AckEnter ack;
    ack.set_code(0);
    *ack.mutable_guid() = INetModule::StructToProtobuf(guid);
    *ack.mutable_object() = INetModule::StructToProtobuf(objectID);
    OnSendToClient(rpc::ACK_ENTER, ack, guid);
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

        Player *player = players_[self.ToString()];
        if (player == nullptr) {
            dout << "Player offline not found!\n";
            return -1;
        }

        player->OnOffline();
        player->offline_time_ = time(nullptr); // 记录离线时间，由Player Manager定时清理已离线的玩家数据
    } else if (CLASS_OBJECT_EVENT::COE_CREATE_LOADDATA == classEvent) {
        dout << "玩家数据对象加载\n";
        // m_data_tail_->StartTrail(self);
        // m_data_tail_->LogObjectData(self);

        LoadDataFromDb(self);
        m_kernel_->SetPropertyInt(self, excel::Player::OnlineTime(), SquickGetTimeS());
    } else if (CLASS_OBJECT_EVENT::COE_CREATE_FINISH == classEvent) {
        /*
        dout << "玩家加载数据完成\n";
        auto it = mxObjectDataCache.find(self);
        if (it != mxObjectDataCache.end()) {
                mxObjectDataCache.erase(it);
        }*/

        // 每3分钟 保存一次玩家数据到数据库
        // m_schedule_->AddSchedule(self, "SaveDataOnTime", this, &PlayerManagerModule::SaveDataOnTime, 180.0f, -1);
    }
    return 0;
}

// 从数据库读取玩家数据
void PlayerManagerModule::LoadDataFromDb(const Guid &self) {
    /*
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
    }*/
}

// 保存玩家数据到数据库
void PlayerManagerModule::SaveDataToDb(const Guid &self) {
    /*
    std::shared_ptr<IObject> xObject = m_kernel_->GetObject(self);
    if (xObject) {
            std::shared_ptr<IPropertyManager> xPropManager = xObject->GetPropertyManager();
            std::shared_ptr<IRecordManager> xRecordManager = xObject->GetRecordManager();
            rpc::PlayerData xDataPack;

            *xDataPack.mutable_object() = INetModule::StructToProtobuf(self);

            *(xDataPack.mutable_property()->mutable_player_id()) = INetModule::StructToProtobuf(self);
            *(xDataPack.mutable_record()->mutable_player_id()) = INetModule::StructToProtobuf(self);

            if (xPropManager) {
                    CommonRedisModule::ConvertPropertyManagerToPB(xPropManager, xDataPack.mutable_property(), false, true);
            }

            if (xRecordManager) {
                    CommonRedisModule::ConvertRecordManagerToPB(xRecordManager, xDataPack.mutable_record(), false, true);
            }
            m_net_client_->SendSuitByPB(ServerType::ST_DB_PROXY, self.GetData(), rpc::DbProxyRPC::REQ_PLAYER_DATA_SAVE, xDataPack);
    }*/
}

// 定时保存玩家数据
int PlayerManagerModule::SaveDataOnTime(const Guid &self, const std::string &name, const float fIntervalTime, const int count) {
    SaveDataToDb(self);
    return 0;
}

bool PlayerManagerModule::Destory() { return true; }

bool PlayerManagerModule::Update() {

    static time_t lut = 0;
    time_t now_time = SquickGetTimeS();
    if (now_time - lut > 5) { // 每五秒检测一次
        lut = now_time;
        UpdateRemoveOfflinePlayers(now_time);
    }

    return true;
}

// 发送数据给客户端，用于给player.cc使用
void PlayerManagerModule::OnSendToClient(const uint16_t msg_id, google::protobuf::Message &xMsg, const Guid &client_id) {
    dout << "发送数据给: " << client_id.ToString() << " msgID: " << msg_id << std::endl;
    m_node_->SendPBToPlayer(msg_id, xMsg, client_id);
}

/*
// virtual  ();
// 发送数据给客户端，用于给player.cc使用
Player* PlayerManagerModule::GetPlayer(const Guid& clientID) {
        auto iter = players_.find(clientID.ToString());
        if (iter == players_.end()) {
                return nullptr;
        }
        return iter->second;
}*/

int PlayerManagerModule::GetPlayerRoomID(const Guid &clientID) {
    auto player = players_[clientID.ToString()];
    if (player != nullptr) {
        return player->GetRoomID();
    }

    return -1;
}

void PlayerManagerModule::SetPlayerRoomID(const Guid &clientID, int groupID) {
    auto player = players_[clientID.ToString()];
    if (player != nullptr) {
        player->SetRoomID(groupID);
    }
}

int PlayerManagerModule::GetPlayerGameplayID(const Guid &clientID) {
    auto player = players_[clientID.ToString()];
    if (player != nullptr) {
        return player->GetGameplayID();
    }
    return -1;
}

void PlayerManagerModule::SetPlayerGameplayID(const Guid &clientID, int groupID) {
    auto player = players_[clientID.ToString()];
    if (player != nullptr) {
        player->SetGameplayID(groupID);
    }
}

void PlayerManagerModule::OnReqPlayerEquipment(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    static int index = 0;
    Guid guid;
    rpc::ReqPlayerEquipment req;
    if (!m_net_->ReceivePB(msg_id, msg, len, req, guid)) {
        return;
    }

    Guid target = INetModule::ProtobufToStruct(req.guid());
    rpc::AckPlayerEquipment ack;
    *ack.mutable_guid() = req.guid();
    ack.set_code(0);
    int array[] = {10001, 10002, 10003};
    unsigned int seed = time(nullptr);
    srand(seed + index);
    index += 99;
    int glove = array[rand() % 3];
    srand(seed + index);
    index += 199;
    if (index > 10000) {
        index = 0;
    }
    int mask = array[rand() % 3];
    ack.set_glove(glove);
    dout << " 玩家装备 : " + target.ToString() << "  glove: " << glove << " mask: " << mask << std::endl;
    ack.set_mask(mask);
    OnSendToClient(rpc::LobbyPlayerRPC::ACK_PLAYER_EQUIPMENT, ack, guid);
}

void PlayerManagerModule::OnReqPlayerData(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid guid;
    rpc::ReqPlayerEquipment req;
    rpc::AckPlayerData ack;
    do {

        if (!m_net_->ReceivePB(msg_id, msg, len, req, guid)) {
            ack.set_code(1);
            break;
        }

        auto iter = players_.find(guid.ToString());
        if (iter == players_.end()) {
            ack.set_code(2);
            break;
        }
        auto player = iter->second;
        ack.set_code(0);
    } while (false);
    OnSendToClient(rpc::LobbyPlayerRPC::ACK_PLAYER_DATA, ack, guid);
}

void PlayerManagerModule::OnReqPlayerLeave(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid guid;
    rpc::ReqLeave xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, guid)) {
        return;
    }
    dout << "玩家离线: " << guid.ToString() << std::endl;

    if (guid.IsNull()) {
        return;
    }

    Offline p;
    int gameplay_id = GetPlayerGameplayID(guid);
    p.type = Offline::LobbyOffline;
    if (gameplay_id != -1) {
        p.type = Offline::PlayingOffline;
    }
    p.time = SquickGetTimeS();
    offline_players_[guid.ToString()] = p;
}

void PlayerManagerModule::UpdateRemoveOfflinePlayers(time_t now_time) {
    std::vector<std::unordered_map<string, Offline>::iterator> expired;
    if (offline_players_.size() > 0) {
        for (auto iter = offline_players_.begin(); iter != offline_players_.end(); ++iter) {
            auto &guid = iter->first;
            auto &data = iter->second;
            // 过期
            if (data.type == Offline::LobbyOffline) {

                if (now_time - data.time > 14) { // 如果是处于大厅，给予15秒进行重连
                    // 退出room
                    // m_room_->RoomQuit(guid);
                    expired.push_back(iter);
                }
            } else if (data.type == Offline::PlayingOffline) {
                if (now_time - data.time > 59) { // 如果是处于游玩中，给予60秒进行通过房间重新加入游戏
                    // 退出gameplay
                    // m_gameplay_manager_->GameplayPlayerQuit(guid); // 立即退出 gameplay
                    // 退出room
                    // m_room_->RoomQuit(guid);
                    expired.push_back(iter);
                }
            }
        }
    }

    // 销毁玩家
    if (expired.size() > 0) {
        for (auto &iter : expired) {
            auto &guid = iter->first;
            auto &data = iter->second;

            // 从 players_ 删除
            auto piter = players_.find(guid);
            if (piter != players_.end()) {
                players_.erase(piter);
            }

            dout << "该玩家已销毁: " << guid << "所处状态: " << data.type << std::endl;
            // m_server_->RemovePlayerProxyInfo(guid);
            offline_players_.erase(iter);
        }
    }
}

void PlayerManagerModule::OnReqPlayerReconnect(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

void PlayerManagerModule::OnRecv(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    rpc::MsgBase xMsg;
    if (!xMsg.ParseFromArray(msg, len)) {
        char szData[MAX_PATH] = {0};
        NFSPRINTF(szData, MAX_PATH, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msg_id);
        return;
    }

    Guid clientID = m_net_->ProtobufToStruct(xMsg.player_id());
    auto iter = players_.find(clientID.ToString());
    if (iter == players_.end()) {
        dout << "不存在该 player: msg_id: " << msg_id << std::endl;
        return;
    }

    auto player = iter->second;
    if (player != nullptr) {
        GAME_PLAY_RECEIVE_FUNCTOR_PTR &ptr = GetCallback(msg_id, clientID.ToString());
        if (ptr != nullptr) {
            GAME_PLAY_RECEIVE_FUNCTOR *pFunc = ptr.get();
            pFunc->operator()(clientID, msg_id, xMsg.msg_data());
        } else {
            dout << "不存在该 callback! msg_id: " << msg_id << std::endl;
        }
    } else {
        dout << "不存在该 player: " << clientID.ToString() << " msg_id: " << msg_id << std::endl;
        // 不存在该player，可能已销毁
    }
}

} // namespace lobby::player