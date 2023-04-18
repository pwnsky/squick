#include "room_module.h"
// #include <squick/plugin/lua/export.h>
// #include <third_party/nlohmann/json.hpp>
#include <third_party/common/crc32.hpp>

namespace game::room {
bool RoomModule::Start() {
    m_net_ = pm_->FindModule<INetModule>();
    m_lua_script_ = pm_->FindModule<ILuaScriptModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    // m_pGameServerNet_ServerModule = pm_->FindModule<IGameServerNet_ServerModule>();
    // m_player_manager_ = pm_->FindModule<IPlayerManagerModule>();
    // m_gameplay_manager_ = pm_->FindModule<play::IGameplayManagerModule>();
    return true;
}

bool RoomModule::Destory() { return true; }

bool RoomModule::AfterStart() {
    m_net_->AddReceiveCallBack(rpc::GameRPC::REQ_ROOM_CREATE, this, &RoomModule::OnReqRoomCreate);
    m_net_->AddReceiveCallBack(rpc::GameRPC::REQ_ROOM_LIST, this, &RoomModule::OnReqRoomList);
    m_net_->AddReceiveCallBack(rpc::GameRPC::REQ_ROOM_DETAILS, this, &RoomModule::OnReqRoomDetails);
    m_net_->AddReceiveCallBack(rpc::GameRPC::REQ_ROOM_JOIN, this, &RoomModule::OnReqRoomJoin);
    m_net_->AddReceiveCallBack(rpc::GameRPC::REQ_ROOM_QUIT, this, &RoomModule::OnReqRoomQuit);
    m_net_->AddReceiveCallBack(rpc::GameRPC::REQ_ROOM_PLAYER_EVENT, this, &RoomModule::OnReqRoomPlayerEvent);

    m_net_->AddReceiveCallBack(rpc::GameRPC::REQ_ROOM_GAME_PLAY_START, this, &RoomModule::OnReqRoomGamePlayStart);

    m_net_->AddReceiveCallBack(rpc::GameplayManagerRPC::REQ_GAMEPLAY_DATA, this, &RoomModule::OnReqGameplayData);
    m_net_->AddReceiveCallBack(rpc::GameplayManagerRPC::REQ_GAMEPLAY_PREPARED, this, &RoomModule::OnReqGameplayPrepared);

#ifdef SQUICK_DEV
    // 默认创建一个开发测试房间
    CreateDevRoom();
#endif //  SQUICK_DEV

    return true;
}

bool RoomModule::Update() { return true; }
void RoomModule::CreateDevRoom() {
    int room_id = 0;
    auto room = new Room();
    room->id = 0;
    room->name = "开发测试房间";
    room->max_players = 20;                            // 默认20人
    room->nplayers = 0;                                // 当前房间人数
    room->status = rpc::RoomStatus::ROOM_GAME_PLAYING; // 已开始游戏

    room->game.id = 1;
    room->game.ip = "127.0.0.1";
    room->game.port = 7777;
    room->game.key = "dev_key";

    rooms_[room_id] = room;
}

// 创建房间时，房主申请进入一个PVP场景创建一个公共组，其他玩家可获取这个group id
void RoomModule::OnReqRoomCreate(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << " OnReqRoomCreate: \n";
    rpc::ReqRoomCreate req;
    rpc::AckRoomCreate ack;
    ack.set_code(0);
    ack.set_room_id(-1);
    Guid player;
    do {
        if (!m_net_->ReceivePB(msg_id, msg, len, req, player)) {
            ack.set_code(1);
            break;
        }

        int old_room_id = m_player_manager_->GetPlayerRoomID(player);
        if (old_room_id != -1) {
            dout << "不能创建房间, 该玩家已加入房间 " << old_room_id << " \n";
            ack.set_code(2);
            break;
        }

        // 生成组ID
        const int room_id = SquickGetTimeMSEx() & 0x7fffffff; // 在 游戏 场景中申请一个roomID
        dout << "Room 申请新的 room id: " << room_id << " \n";
        m_player_manager_->SetPlayerRoomID(player, room_id);

        auto room = new Room;
        // .......
        room->id = room_id;
        room->name = req.name();
        room->max_players = 10;                        // 默认10人
        room->nplayers = 1;                            // 当前房间人数
        room->status = rpc::RoomStatus::ROOM_PREPARED; // 默认房间已准备好,方便测试
        room->owner = player;

        RoomPlyaer p;
        p.guid = player;
        p.name = "player_name";
        room->players.push_back(p);

        rooms_[room_id] = room;

        ack.set_room_id(room_id);

    } while (false);
    dout << "创建房间返回 code: " << ack.code() << std::endl;
    SendToPlayer(rpc::ACK_ROOM_CREATE, ack, player);
}

// 请求加入房间
void RoomModule::OnReqRoomJoin(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << "OnReqRoomJoin\n";
    Guid player;
    rpc::ReqRoomJoin xMsg;
    rpc::AckRoomJoin ack;

    do {
        if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, player)) {
            ack.set_code(1);
            break;
        }

        *ack.mutable_player() = INetModule::StructToProtobuf(player);
        int room_id = xMsg.room_id();
        // 检查房间状态
        auto room = GetRoom(room_id);
        if (room == nullptr) {
            // 房间不存在
            dout << "Error: room_id" << room_id << "房间不存在\n";
            ack.set_code(2);
            break;
        }

        if (room->nplayers + 1 > room->max_players) {
            dout << "房间已满\n";
            ack.set_code(3);
            return;
        }

        // 检查当前玩家是否绑定了房间
        int old_room = m_player_manager_->GetPlayerRoomID(player);
        if (old_room != -1) {
            dout << "已加入房间 " << old_room << "\n";
            ack.set_code(4);
            return;
        }

        ack.set_code(0);
        // 将当前玩家加入到房间
        m_player_manager_->SetPlayerRoomID(player, room_id);

        int nplayers = room->nplayers + 1;
        room->nplayers = nplayers;
        RoomPlyaer p;
        p.guid = player;
        p.name = "join player";
        room->players.push_back(p);
        BroadcastToPlayers(rpc::GameRPC::ACK_ROOM_JOIN, ack, room_id);
        return;
    } while (false);
    m_server_->SendMsgPBToProxy(rpc::ACK_ROOM_JOIN, ack, player);
}

// 获取房间列表
void RoomModule::OnReqRoomList(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << "OnReqRoomList\n";

    Guid clientID;
    rpc::ReqRoomList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, clientID)) {
        return;
    }

    int start = xMsg.start();
    int limit = xMsg.limit();
    if (start >= rooms_.size()) {
        dout << "bad request: "
             << "\n";
        return;
    }

    rpc::AckRoomList ack;
    rpc::RoomSimple *rs;

    int i = 0;
    for (auto iter = rooms_.begin(); iter != rooms_.end(); ++iter) {
        if (i > 100) {
            break;
        }
        auto room = iter->second;
        if (room == nullptr) {
            // 出现key存在,value为空情况，继续遍历，遍历完成后释放该key
            dout << "rooms遍历, 存在value为 nullptr, key: " << iter->first << std::endl;
            continue;
        }

        rs = ack.add_list();
        rs->set_name(room->name);
        rs->set_id(room->id);
        rs->set_status(room->status);
        rs->set_max_players(room->max_players);
        rs->set_nplayers(room->nplayers);
        // dout << "遍历: " << iter->second->name() << std::endl;
    }

    m_pGameServerNet_ServerModule->SendMsgPBToProxy(rpc::ACK_ROOM_LIST, ack, clientID);
}

void RoomModule::OnReqRoomDetails(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << "OnReqRoomDetails\n";
    Guid clientID;
    rpc::ReqRoomDetails xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, clientID)) {
        return;
    }

    // 检查房间状态
    int room_id = xMsg.room_id();
    auto room = GetRoom(room_id);
    if (room == nullptr) {
        // 房间不存在
        dout << "Error: room_id" << room_id << "房间不存在\n";
        return;
    }

    rpc::AckRoomDetails ack;
    auto rd = GetRoomDetails(room_id);
    *ack.mutable_room() = rd;
    SendToPlayer(rpc::GameRPC::ACK_ROOM_DETAILS, ack, clientID);
}
Room *RoomModule::GetRoom(int room_id) {
    auto iter = rooms_.find(room_id);
    if (iter == rooms_.end()) {
        return nullptr;
    }
    return iter->second;
}

rpc::RoomDetails RoomModule::GetRoomDetails(int room_id) {
    auto room = GetRoom(room_id);
    rpc::RoomDetails r;
    r.set_id(room->id);
    *r.mutable_owner() = INetModule::StructToProtobuf(room->owner);
    r.set_nplayers(room->nplayers);
    r.set_name(room->name);
    r.set_max_players(room->max_players);
    r.set_status(room->status);

    for (auto &p : room->players) {
        auto n = r.add_players();
        *n->mutable_guid() = INetModule::StructToProtobuf(p.guid);
        n->set_name(p.name);
        n->set_status(p.status);
    }

    auto g = new rpc::RoomGamePlay;
    g->set_ip(room->game.ip);
    g->set_port(room->game.port);
    g->set_id(room->game.id);
    g->set_key(room->game.key);
    g->set_mode(room->game.mode);
    g->set_name(room->game.name);
    g->set_scene(room->game.scene);
    r.set_allocated_game_play(g);
    return r;
}

void RoomModule::OnReqRoomQuit(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << "OnReqRoomQuit\n";
    Guid player;
    rpc::ReqRoomQuit xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, player)) {
        return;
    }

    // 检查房间状态
    int room_id = xMsg.room_id();

    rpc::AckRoomQuit ack;
    *ack.mutable_player() = INetModule::StructToProtobuf(player);
    if (RoomQuit(player) == true) {
        ack.set_code(0);
        BroadcastToPlayers(rpc::ACK_ROOM_QUIT, ack, room_id);
        // return;
    } else {
        ack.set_code(1);
    }
    m_pGameServerNet_ServerModule->SendMsgPBToProxy(rpc::ACK_ROOM_QUIT, ack, player); // 发送给退出者
}

bool RoomModule::RoomQuit(const Guid &player) {

    int roomID = m_player_manager_->GetPlayerRoomID(player);
    if (roomID == -1) {
        return true;
    }

    auto room = GetRoom(roomID);
    if (room == nullptr) {
        m_player_manager_->SetPlayerRoomID(player, -1);
        // 房间不存在
        dout << "Error: roomID" << roomID << "房间不存在\n";
        return false;
    }

    m_player_manager_->SetPlayerRoomID(player, -1);

    for (auto iter = room->players.begin(); iter != room->players.end(); ++iter) {
        if (iter->guid == player) {
            room->players.erase(iter);
            room->nplayers -= 1;
            break;
        }
    }

    // 检查是否能删除房间
    if (room->nplayers < 1) {
        dout << "房间自动销毁\n";
        // 销毁当前房间
        delete rooms_[roomID];
        auto iter = rooms_.find(roomID);
        if (iter != rooms_.end()) {
            rooms_.erase(iter);
        } else {
            dout << "房间销毁 错误\n";
            return false;
        }
    }

    m_player_manager_->SetPlayerRoomID(player, -1);
    return true;
}

void RoomModule::OnReqRoomPlayerEvent(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) { dout << "OnReqRoomQuit\n"; }

// 开始游戏
void RoomModule::OnReqRoomGamePlayStart(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << "OnReqStartGame\n";
    Guid player;
    rpc::ReqRoomGamePlayStart xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, player)) {
        return;
    }

    // 检查房间状态
    int room_id = xMsg.room_id();
    auto room = GetRoom(room_id);
    if (room == nullptr) {
        // 房间不存在
        dout << "Error: room_id" << room_id << "房间不存在\n";
        return;
    }

    switch (room->status) {
    case rpc::ROOM_PREPARING: {
        dout << "房间未准备中!";
    } break;

    case rpc::ROOM_PREPARED: {
        if (room->owner == player) {
            // if(room->game_play().id() != -1)
            dout << "开始游戏!";
            // 创建游戏
            // 生成instance_id和instance_key
            int instance_id = room->id;
            string instance_key = std::to_string(SquickProtocol::CRC32(std::to_string(time(nullptr) + 0x418894113)));

            // 将信息保存在房间里
            room->game.id = instance_id;
            room->game.key = instance_key;

#ifdef SINGLE_GAMEPLAY
            // 启动 独立的Gameplay服务器

            m_gameplay_manager_->SingleGameplayCreate(instance_id, instance_key);
#else
            // 启动 融合在game服务器上的gameplay
            m_gameplay_manager_->GameplayCreate(instance_id, instance_key);
#endif // SINGLE_GAMEPLAY

        } else {
            dout << "没有权限开始游戏! room_id " << room_id << " player_id: " << player.ToString() << "\n";
        }
    } break;

    case rpc::ROOM_GAME_PLAYING: {
        dout << "玩家加入游戏!\n";
        // 加入游戏
        rpc::AckRoomGamePlayStart ack;
        ack.set_code(0);
        rpc::RoomGamePlay *gamePlay = new rpc::RoomGamePlay();
        gamePlay->set_ip(room->game.ip);
        gamePlay->set_port(room->game.port);
        gamePlay->set_key(room->game.key);
        gamePlay->set_name(room->game.name);
        ack.set_allocated_game_play(gamePlay);
        m_pGameServerNet_ServerModule->SendMsgPBToProxy(rpc::GameRPC::ACK_ROOM_GAME_PLAY_START, ack, player);
    } break;
    }
}

// 来自Gameplay Server初始化游戏数据
void RoomModule::OnReqGameplayData(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << "OnReqGameplayInit\n";
    Guid clientID;
    rpc::ReqGameplayData xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, clientID)) {
        return;
    }

    int room_id = xMsg.id();
    auto room = GetRoom(room_id);
    if (room == nullptr) {
        // 房间不存在
        dout << "Error: room_id" << room_id << "房间不存在\n";
        return;
    }

#ifndef SQUICK_DEV
    // 检查id 和key
    if (room->server().instance_id() != xMsg.instance_id() || room->server().instance_key() != xMsg.instance_key()) {
        dout << "Error: Id or key is error!\n";
        return;
    }
#endif // !SQUICK_DEV

    rpc::AckRoomDetails ack;
    auto rd = GetRoomDetails(room_id);
    *ack.mutable_room() = rd;

    // 发送房间内详细数据给Gameplay服务器
    m_pGameServerNet_ServerModule->SendPBToGameplay(rpc::ACK_GAMEPLAY_DATA, ack, clientID);
}

// PVP Server初始化游戏数据完成
void RoomModule::OnReqGameplayPrepared(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << "OnReqGamePlayPrepared\n";
    Guid clientID;
    rpc::ReqGameplayPrepared xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, clientID)) {
        return;
    }
    int roomID = xMsg.id();
    auto room = GetRoom(roomID);
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
    auto room = GetRoom(roomID);
    if (room == nullptr) {
        // 房间不存在
        dout << "Error: room_id" << roomID << "房间不存在\n";
        return;
    }

    rpc::AckRoomGamePlayStart ack;
    ack.set_code(0);

    // 保存Gameplay服务器信息
    room->status = rpc::ROOM_GAME_PLAYING;
    room->game.ip = ip;
    room->game.port = port;
    room->game.name = name;

    rpc::RoomGamePlay *server = new rpc::RoomGamePlay();
    server->set_ip(ip);
    server->set_port(port);
    server->set_name(name);
    server->set_key(room->game.key);
    ack.set_allocated_game_play(server);

    // 广播通知房间内的所有玩家加入 Game Play 服务器
    BroadcastToPlayers(rpc::ACK_ROOM_GAME_PLAY_START, ack, roomID);
}

void RoomModule::SendToPlayer(const uint16_t msg_id, google::protobuf::Message &xMsg, const Guid &player) {
    m_pGameServerNet_ServerModule->SendMsgPBToProxy(msg_id, xMsg, player);
}

// 广播发送给房间内所有玩家
void RoomModule::BroadcastToPlayers(const uint16_t msg_id, google::protobuf::Message &xMsg, int roomID) {
    dout << "广播房间内所有玩家, Room ID: " << roomID << std::endl;
    // xMsg.instance_id()
    auto room = GetRoom(roomID);
    if (room == nullptr) {
        // 房间不存在
        dout << "Error: room_id" << roomID << "房间不存在\n";
        return;
    }

    for (auto const &player : room->players) {
        dout << "Broad cast to: " << player.guid.ToString() << std::endl;
        m_pGameServerNet_ServerModule->SendMsgPBToProxy(msg_id, xMsg, player.guid);
    }
}

Room *RoomModule::GetRoomByID(int room_id) { return GetRoom(room_id); }

} // namespace game::lobby
