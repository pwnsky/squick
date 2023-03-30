#include "room_module.h"
// #include <squick/plugin/lua/export.h>
// #include <third_party/nlohmann/json.hpp>
#include <third_party/common/crc32.hpp>

namespace game::player {
bool RoomModule::Start() {
    m_pNetModule = pPluginManager->FindModule<INetModule>();
    m_pLuaScriptModule = pPluginManager->FindModule<ILuaScriptModule>();
    m_pLogModule = pPluginManager->FindModule<ILogModule>();
    m_pGameServerNet_ServerModule = pPluginManager->FindModule<IGameServerNet_ServerModule>();
    m_pPlayerManagerModule = pPluginManager->FindModule<IPlayerManagerModule>();
    m_pGameplayManagerModule = pPluginManager->FindModule<play::IGameplayManagerModule>();
    return true;
}

bool RoomModule::Destory() { return true; }

bool RoomModule::AfterStart() {
    m_pNetModule->AddReceiveCallBack(SquickStruct::GameLobbyRPC::REQ_ROOM_CREATE, this, &RoomModule::OnReqRoomCreate);
    m_pNetModule->AddReceiveCallBack(SquickStruct::GameLobbyRPC::REQ_ROOM_LIST, this, &RoomModule::OnReqRoomList);
    m_pNetModule->AddReceiveCallBack(SquickStruct::GameLobbyRPC::REQ_ROOM_DETAILS, this, &RoomModule::OnReqRoomDetails);
    m_pNetModule->AddReceiveCallBack(SquickStruct::GameLobbyRPC::REQ_ROOM_JOIN, this, &RoomModule::OnReqRoomJoin);
    m_pNetModule->AddReceiveCallBack(SquickStruct::GameLobbyRPC::REQ_ROOM_QUIT, this, &RoomModule::OnReqRoomQuit);
    m_pNetModule->AddReceiveCallBack(SquickStruct::GameLobbyRPC::REQ_ROOM_PLAYER_EVENT, this, &RoomModule::OnReqRoomPlayerEvent);

    m_pNetModule->AddReceiveCallBack(SquickStruct::GameLobbyRPC::REQ_ROOM_GAME_PLAY_START, this, &RoomModule::OnReqRoomGamePlayStart);

    m_pNetModule->AddReceiveCallBack(SquickStruct::GameplayManagerRPC::REQ_GAMEPLAY_DATA, this, &RoomModule::OnReqGameplayData);
    m_pNetModule->AddReceiveCallBack(SquickStruct::GameplayManagerRPC::REQ_GAMEPLAY_PREPARED, this, &RoomModule::OnReqGameplayPrepared);


#ifdef SQUICK_DEV
    // 默认创建一个开发测试房间
    CreateDevRoom();
#endif //  SQUICK_DEV

    return true;
}

bool RoomModule::Update() { return true; }
void RoomModule::CreateDevRoom() {
    int room_id = 0;
    auto roomDetails = new SquickStruct::RoomDetails();
    roomDetails->set_id(room_id);
    roomDetails->set_name("开发测试房间");
    roomDetails->set_max_players(20);                                     // 默认20人
    roomDetails->set_nplayers(0);                                         // 当前房间人数
    roomDetails->set_status(SquickStruct::RoomStatus::ROOM_GAME_PLAYING); // 已开始游戏
    roomDetails->set_allocated_owner(new SquickStruct::Ident(m_pNetModule->StructToProtobuf(Guid())));

    // roomDetails->set_map_id(1); // 默认地图

    SquickStruct::RoomGamePlay *gamePlay = new SquickStruct::RoomGamePlay();
    gamePlay->set_ip("192.168.0.142");
    gamePlay->set_port(7777);
    gamePlay->set_id(1);
    gamePlay->set_key("dev_key");
    roomDetails->set_allocated_game_play(gamePlay);
    m_rooms[room_id] = roomDetails;
}

// 创建房间时，房主申请进入一个PVP场景创建一个公共组，其他玩家可获取这个group id
void RoomModule::OnReqRoomCreate(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    int sceneId = 3; // 游戏 场景
    dout << " OnReqRoomCreate: \n";
    Guid clientID;
    SquickStruct::ReqRoomCreate xMsg;
    if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, clientID)) {
        return;
    }

    int old_room_id = m_pPlayerManagerModule->GetPlayerRoomID(clientID);
    if(old_room_id != -1) {
        dout << "不能创建房间, 该玩家已加入房间 " << old_room_id << " \n";
        return;
    }

    // 生成组ID
    const int roomID = SquickGetTimeMSEx() & 0x7fffffff; // 在 游戏 场景中申请一个roomID
    dout << "Room 申请新的 room id: " << roomID << " \n";
    m_pPlayerManagerModule->SetPlayerRoomID(clientID, roomID);

    auto roomDetails = new SquickStruct::RoomDetails();
    // .......
    roomDetails->set_id(roomID);
    roomDetails->set_name(xMsg.name());
    roomDetails->set_max_players(10);                                 // 默认10人
    roomDetails->set_nplayers(1);                                     // 当前房间人数
    roomDetails->set_status(SquickStruct::RoomStatus::ROOM_PREPARED); // 默认房间已准备好,方便测试

    roomDetails->set_allocated_owner(new SquickStruct::Ident(m_pNetModule->StructToProtobuf(clientID)));
    SquickStruct::RoomPlayer *player = roomDetails->add_players();
    player->set_allocated_guid(new SquickStruct::Ident(m_pNetModule->StructToProtobuf(clientID)));
    player->set_name("player_name");
    player->set_status(SquickStruct::RoomPlayerStatus::ROOM_PLAYER_STATUS_NOT_PREPARE);

    SquickStruct::RoomGamePlay *gamePlay = new SquickStruct::RoomGamePlay();
    gamePlay->set_scene(sceneId);
    gamePlay->set_mode(0);
    roomDetails->set_allocated_game_play(gamePlay);

    m_rooms[roomID] = roomDetails;

    // 通知玩家创建房间成功
    SquickStruct::AckRoomCreate ack;
    ack.set_room_id(roomID);
    ack.set_code(0);
    SendToPlayer(SquickStruct::ACK_ROOM_CREATE, ack, clientID);
}

// 请求加入房间
void RoomModule::OnReqRoomJoin(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    dout << "OnReqRoomJoin\n";
    Guid clientID;
    SquickStruct::ReqRoomJoin xMsg;
    if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, clientID)) {
        return;
    }

    int room_id = xMsg.room_id();
    // 检查房间状态
    auto room = m_rooms[room_id];
    if (room == nullptr) {
        // 房间不存在
        dout << "Error: room_id" << room_id << "房间不存在\n";
        return;
    }

    if (room->nplayers() + 1 > room->max_players()) {
        dout << "房间已满\n";
        return;
    }

    // 检查当前玩家是否绑定了房间
    int old_room = m_pPlayerManagerModule->GetPlayerRoomID(clientID);
    if (old_room != -1) {
        dout << "已加入房间 " << old_room << "\n";
        return;
    }

    // 将当前玩家加入到房间
    m_pPlayerManagerModule->SetPlayerRoomID(clientID, room_id);

    int nplayers = room->nplayers() + 1;
    room->set_nplayers(nplayers);
    SquickStruct::RoomPlayer *player = room->add_players();
    player->set_name("join player");
    player->set_status(SquickStruct::ROOM_PLAYER_STATUS_NOT_PREPARE);
    player->set_allocated_guid(new SquickStruct::Ident(m_pNetModule->StructToProtobuf(clientID)));

    SquickStruct::AckRoomJoin ack;
    ack.set_code(0);
    m_pGameServerNet_ServerModule->SendMsgPBToProxy(SquickStruct::ACK_ROOM_JOIN, ack, clientID);
}

// 获取房间列表
void RoomModule::OnReqRoomList(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    dout << "OnReqRoomList\n";

    Guid clientID;
    SquickStruct::ReqRoomList xMsg;
    if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, clientID)) {
        return;
    }

    int start = xMsg.start();
    int limit = xMsg.limit();
    if (start >= m_rooms.size()) {
        dout << "bad request: "
             << "\n";
        return;
    }

    SquickStruct::AckRoomList ack;
    SquickStruct::RoomSimple *room;

    int i = 0;
    for (auto iter = m_rooms.begin(); iter != m_rooms.end(); ++iter) {
        if (i > 100) {
            break;
        }
        auto roomDetails = iter->second;
        if (roomDetails == nullptr) {
            // 出现key存在,value为空情况，继续遍历，遍历完成后释放该key
            dout << "rooms遍历, 存在value为 nullptr, key: " << iter->first << std::endl;
            continue;
        }

        room = ack.add_list();
        room->set_name(roomDetails->name());
        room->set_id(roomDetails->id());
        room->set_status(roomDetails->status());
        room->set_max_players(roomDetails->max_players());
        room->set_nplayers(roomDetails->nplayers());
        // dout << "遍历: " << iter->second->name() << std::endl;
    }

    m_pGameServerNet_ServerModule->SendMsgPBToProxy(SquickStruct::ACK_ROOM_LIST, ack, clientID);
}

void RoomModule::OnReqRoomDetails(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    dout << "OnReqRoomDetails\n";
    Guid clientID;
    SquickStruct::ReqRoomDetails xMsg;
    if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, clientID)) {
        return;
    }

    // 检查房间状态
    int room_id = xMsg.room_id();
    auto room = m_rooms[room_id];
    if (room == nullptr) {
        // 房间不存在
        dout << "Error: room_id" << room_id << "房间不存在\n";
        return;
    }

    SquickStruct::AckRoomDetails ack;
    *ack.mutable_room() = *room;
    SendToPlayer(SquickStruct::GameLobbyRPC::ACK_ROOM_DETAILS, ack, clientID);
}

void RoomModule::OnReqRoomQuit(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    dout << "OnReqRoomQuit\n";
    Guid clientID;
    SquickStruct::ReqRoomQuit xMsg;
    if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, clientID)) {
        return;
    }

    // 检查房间状态
    int room_id = xMsg.room_id();

    SquickStruct::AckRoomQuit ack;
    if (RoomQuit(clientID) == true) {
        ack.set_code(0);
    } else {
        ack.set_code(1);
    }
    m_pGameServerNet_ServerModule->SendMsgPBToProxy(SquickStruct::ACK_ROOM_QUIT, ack, clientID);
}

bool RoomModule::RoomQuit(const Guid &clientID) {

    int roomID = m_pPlayerManagerModule->GetPlayerRoomID(clientID);
    if (roomID == -1) {
        return true;
    }

    auto room = m_rooms[roomID];
    if (room == nullptr) {
        m_pPlayerManagerModule->SetPlayerRoomID(clientID, -1);
        // 房间不存在
        dout << "Error: roomID" << roomID << "房间不存在\n";
        return false;
    }

    m_pPlayerManagerModule->SetPlayerRoomID(clientID, -1);

    for (auto iter = room->mutable_players()->begin(); iter != room->mutable_players()->end(); ++iter) {
        Guid guid = m_pNetModule->ProtobufToStruct(iter->guid());
        if (guid == clientID) {
            room->mutable_players()->erase(iter);
            int nplayers = room->nplayers() - 1;
            room->set_nplayers(nplayers);
            break;
        }
    }

    // 检查是否能删除房间
    if (room->nplayers() < 1) {
        dout << "房间自动销毁\n";
        // 销毁当前房间
        delete m_rooms[roomID];
        auto iter = m_rooms.find(roomID);
        if (iter != m_rooms.end()) {
            m_rooms.erase(iter);
        } else {
            dout << "房间销毁 错误\n";
            return false;
        }
    }

    m_pPlayerManagerModule->SetPlayerRoomID(clientID, -1);
    return true;
}

void RoomModule::OnReqRoomPlayerEvent(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) { dout << "OnReqRoomQuit\n"; }

// 开始游戏
void RoomModule::OnReqRoomGamePlayStart(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    dout << "OnReqStartGame\n";
    Guid clientID;
    SquickStruct::ReqRoomGamePlayStart xMsg;
    if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, clientID)) {
        return;
    }

    // 检查房间状态
    int room_id = xMsg.room_id();
    auto room = m_rooms[room_id];
    if (room == nullptr) {
        // 房间不存在
        dout << "Error: room_id" << room_id << "房间不存在\n";
        return;
    }

    Guid owner = m_pNetModule->ProtobufToStruct(room->owner());

    switch (room->status()) {
    case SquickStruct::ROOM_PREPARING: {
        dout << "房间未准备中!";
    } break;

    case SquickStruct::ROOM_PREPARED: {
        if (owner == clientID) {
            //if(room->game_play().id() != -1) 
            dout << "开始游戏!";
            // 创建游戏
            // 生成instance_id和instance_key
            int instance_id = room->id();
            string instance_key = std::to_string(SquickProtocol::CRC32(std::to_string(time(nullptr) + 0x418894113)));

            // 将信息保存在房间里
            room->mutable_game_play()->set_id(instance_id);
            room->mutable_game_play()->set_key(instance_key);

#ifdef SINGLE_GAMEPLAY
            // 启动 独立的Gameplay服务器
            
            m_pGameplayManagerModule->SingleGameplayCreate(instance_id, instance_key);
#else
            // 启动 融合在game服务器上的gameplay
            m_pGameplayManagerModule->GameplayCreate(instance_id, instance_key);
#endif // SINGLE_GAMEPLAY

        } else {
            dout << "没有权限开始游戏! room_id " << room_id << " client_id: " << clientID.ToString() << "\n";
        }
    } break;

    case SquickStruct::ROOM_GAME_PLAYING: {
        dout << "玩家加入游戏!\n";
        // 加入游戏
        SquickStruct::AckRoomGamePlayStart ack;
        ack.set_code(0);
        SquickStruct::RoomGamePlay play = room->game_play();
        SquickStruct::RoomGamePlay *gamePlay = new SquickStruct::RoomGamePlay();
        gamePlay->set_ip(play.ip());
        gamePlay->set_port(play.port());
        gamePlay->set_key(play.key());
        gamePlay->set_name(play.name());
        ack.set_allocated_game_play(gamePlay);
        m_pGameServerNet_ServerModule->SendMsgPBToProxy(SquickStruct::GameLobbyRPC::ACK_ROOM_GAME_PLAY_START, ack, clientID);
    } break;
    }
}

// 来自Gameplay Server初始化游戏数据
void RoomModule::OnReqGameplayData(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    dout << "OnReqGameplayInit\n";
    Guid clientID;
    SquickStruct::ReqGameplayData xMsg;
    if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, clientID)) {
        return;
    }

    int roomID = xMsg.id();
    auto room = m_rooms[roomID];
    if (room == nullptr) {
        // 房间不存在
        dout << "Error: room_id" << roomID << "房间不存在\n";
        return;
    }

#ifndef SQUICK_DEV
    // 检查id 和key
    if (room->server().instance_id() != xMsg.instance_id() || room->server().instance_key() != xMsg.instance_key()) {
        dout << "Error: Id or key is error!\n";
        return;
    }
#endif // !SQUICK_DEV

    // 发送房间内详细数据给Gameplay服务器
    m_pGameServerNet_ServerModule->SendMsgPBToGameplay(SquickStruct::ACK_GAMEPLAY_DATA, *room, clientID);
}

// PVP Server初始化游戏数据完成
void RoomModule::OnReqGameplayPrepared(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    dout << "OnReqGamePlayPrepared\n";
    Guid clientID;
    SquickStruct::ReqGameplayPrepared xMsg;
    if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, clientID)) {
        return;
    }
    int roomID = xMsg.id();
    auto room = m_rooms[roomID];
    if (room == nullptr) {
        // 房间不存在
        dout << "Error: room_id" << roomID << "房间不存在\n";
        return;
    }
#ifndef SQUICK_DEV
    if (room->server().instance_id() != xMsg.instance_id() || room->server().instance_key() != xMsg.instance_key()) {
        dout << "Error: Id or key is error!\n";
        return;
    }
#endif

    dout << "game Play server: " << xMsg.ip() << ":" << xMsg.port() << std::endl;
    GamePlayPrepared(roomID, xMsg.name(), xMsg.ip(), xMsg.port());
}

void RoomModule::GamePlayPrepared(int room_id, const string &name, const string &ip, int port) {
    dout << "GamePlayPrepared: " << std::endl;
    int roomID = room_id;
    auto room = m_rooms[roomID];
    if (room == nullptr) {
        // 房间不存在
        dout << "Error: room_id" << roomID << "房间不存在\n";
        return;
    }

    SquickStruct::AckRoomGamePlayStart ack;
    ack.set_code(0);

    // 保存PVP服务器信息
    room->set_status(SquickStruct::ROOM_GAME_PLAYING);
    room->mutable_game_play()->set_ip(ip);
    room->mutable_game_play()->set_port(port);
    room->mutable_game_play()->set_name(name);

    SquickStruct::RoomGamePlay *server = new SquickStruct::RoomGamePlay();
    server->set_ip(ip);
    server->set_port(port);
    server->set_name(name);
    server->set_key(room->game_play().key());
    ack.set_allocated_game_play(server);

    // 广播通知房间内的所有玩家加入 Game Play 服务器
    BroadcastToPlyaers(SquickStruct::ACK_ROOM_GAME_PLAY_START, ack, roomID);
}

void RoomModule::SendToPlayer(const uint16_t msgID, google::protobuf::Message &xMsg, const Guid &player) {
    m_pGameServerNet_ServerModule->SendMsgPBToProxy(msgID, xMsg, player);
}

// 广播发送给房间内所有玩家
void RoomModule::BroadcastToPlyaers(const uint16_t msgID, google::protobuf::Message &xMsg, int roomID) {
    dout << "广播房间内所有玩家, Room ID: " << roomID << std::endl;
    // xMsg.instance_id()
    auto room = m_rooms[roomID];
    if (room == nullptr) {
        // 房间不存在
        dout << "Error: room_id" << roomID << "房间不存在\n";
        return;
    }

    for (auto const &player : room->players()) {
        Guid clientID = m_pNetModule->ProtobufToStruct(player.guid());
        dout << "Broad cast to: " << clientID.ToString() << std::endl;
        m_pGameServerNet_ServerModule->SendMsgPBToProxy(msgID, xMsg, clientID);
    }
}

SquickStruct::RoomDetails *RoomModule::GetRoomByID(int room_id) { return m_rooms[room_id]; }

} // namespace game::player
