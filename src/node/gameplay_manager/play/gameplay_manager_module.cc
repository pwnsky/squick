

#include "gameplay_manager_module.h"
#include <server/db_proxy/logic/common_redis_module.h>

namespace game::play {

bool GameplayManagerModule::Start() {
    m_element_ = pm_->FindModule<IElementModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_net_ = pm_->FindModule<INetModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_pGameToDBModule = pm_->FindModule<IGameServerToDBModule>();

    m_pGameServerNet_ServerModule = pm_->FindModule<IGameServerNet_ServerModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_schedule_ = pm_->FindModule<IScheduleModule>();
    m_data_tail_ = pm_->FindModule<IDataTailModule>();
    m_scene_ = pm_->FindModule<ISceneModule>();
    m_event_ = pm_->FindModule<IEventModule>();

    m_room_ = pm_->FindModule<lobby::IRoomModule>();
    m_player_manager_ = pm_->FindModule<lobby::IPlayerManagerModule>();
    return true;
}

bool GameplayManagerModule::AfterStart() { return true; }

bool GameplayManagerModule::ReadyUpdate() { return true; }

bool GameplayManagerModule::Destory() { return true; }

bool GameplayManagerModule::Update() {
    static int64_t lastGameplayUpdate = SquickGetTimeMS();
    int64_t nowTime = SquickGetTimeMS();
    if (nowTime - lastGameplayUpdate >= 50) // 20fps刷新
    {
        lastGameplayUpdate = nowTime;
        if (m_gameplay.size() > 0) {
            for (auto &game : m_gameplay) {
                if (game.second == nullptr) {
                    dout << "No this Gameplay: gameplayID: " << game.first << std::endl;
                    gameplayWaitDestroy.push_back(game.first);
                    continue;
                }

                if (game.second->GetStatus() == IGameplay::RUNNING) {
                    game.second->DoUpdate();
                } else if (game.second->GetStatus() == IGameplay::GAMEOVER) {
                    gameplayWaitDestroy.push_back(game.first);
                }
            }
        }

        // 自动释放已经结束的gameplay
        if (gameplayWaitDestroy.size() > 0) {
            for (auto g : gameplayWaitDestroy) {
                GameplayDestroy(g);
            }
            gameplayWaitDestroy.clear();
        }
    }

    return true;
}

bool GameplayManagerModule::GameplayCreate(int id, const string &key) {
    dout << "GamePlay Create!\n";
    if (m_gameplay[id] == nullptr) {
        IGameplay *game = new Gameplay();
        game->DoInit(id, this);
        game->DoAwake();
        game->DoStart();
        m_gameplay[id] = game;
    }
    return true;
}

bool GameplayManagerModule::GameplayDestroy(int id) {
    auto iter = m_gameplay.find(id);
    if (iter != m_gameplay.end()) {
        if (iter->second) {
            iter->second->DoDestroy();
            delete iter->second;
        }
        dout << "Gameplay销毁\n";
        m_gameplay.erase(iter);
        return true;
    } else {
        dout << "Gameplay没有找到\n";
    }
    return false;
}

bool GameplayManagerModule::GameplayPlayerQuit(const Guid &player) {
    dout << "玩家: " << player.ToString() << " quit \n";
    int id = m_player_manager_->GetPlayerGameplayID(player);
    if (id != -1) {
        auto iter = m_gameplay.find(id);
        if (iter == m_gameplay.end()) {
            dout << "No this gameplay: " << id << std::endl;
        }
        auto gameplay = m_gameplay[id];
        if (gameplay != nullptr) {
            gameplay->DoPlayerQuit(player);
            // 检查是否可以销毁该gameplay
            if (gameplay->OnlinePlayerCount() < 1) {
                return GameplayDestroy(id);
            }
        } else {
            dout << "该玩家没有在游戏对局中" << std::endl;
        }
    } else {
        dout << "该玩家没有在游戏对局中" << std::endl;
    }
    return false;
}

bool GameplayManagerModule::SingleGameplayCreate(int id, const string &key) {
    dout << "启动 独立的Gameplay服务器 id: " << id << " key: " << key << std::endl;
    rpc::ReqGameplayCreate xMsg;
    xMsg.set_id(id);
    xMsg.set_key(key);
    xMsg.set_game_id(pm_->GetAppID()); // 获取当前Game ID
    m_pGameServerNet_ServerModule->SendMsgPBToGameplayManager(GameplayManagerRPC::REQ_GAMEPLAY_CREATE, xMsg);
    return true;
}

bool GameplayManagerModule::SingleGameplayDestroy(int id) { return true; }

void GameplayManagerModule::OnRecv(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    rpc::MsgBase xMsg;
    if (!xMsg.ParseFromArray(msg, len)) {
        char szData[MAX_PATH] = {0};
        NFSPRINTF(szData, MAX_PATH, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msg_id);
        return;
    }

    Guid clientID = m_net_->ProtobufToStruct(xMsg.player_id());
    int group_id = m_player_manager_->GetPlayerRoomID(clientID);

    if (group_id == -1)
        return;

    auto iter = m_gameplay.find(group_id);
    if (iter == m_gameplay.end()) {
        dout << "不存在该 group: " << group_id << " msg_id: " << msg_id << std::endl;
        return;
    }

    auto gameplay = iter->second;
    if (gameplay != nullptr) {
        GAME_PLAY_RECEIVE_FUNCTOR_PTR &ptr = GetCallback(msg_id, group_id);
        if (ptr != nullptr) {
            GAME_PLAY_RECEIVE_FUNCTOR *pFunc = ptr.get();
            pFunc->operator()(clientID, msg_id, xMsg.msg_data());
        } else {
            dout << "不存在该 callback! msg_id: " << msg_id << std::endl;
        }

    } else {
        dout << "不存在该 group: " << group_id << " msg_id: " << msg_id << std::endl;
        // 不存在该group，可能已销毁
    }
}

} // namespace game::play