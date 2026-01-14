

#include "game_mgr_module.h"

namespace game::logic {

bool GameMgrModule::Start() {
    m_element_ = pm_->FindModule<IElementModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_net_ = pm_->FindModule<INetModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_node_ = pm_->FindModule<INodeModule>();
    return true;
}

bool GameMgrModule::AfterStart() { return true; }

bool GameMgrModule::ReadyUpdate() { return true; }

bool GameMgrModule::Destroy() { return true; }

bool GameMgrModule::Update() {
    static int64_t last_gameplay_update_time = SquickGetTimeMS();
    int64_t now_time = SquickGetTimeMS();
    if (now_time - last_gameplay_update_time >= 50) // 20fps
    {
        last_gameplay_update_time = now_time;
        if (games_.size() > 0) {
            for (auto &game : games_) {
                if (game.second == nullptr) {
                    dead_games_.push_back(game.first);
                    continue;
                }

                if (game.second->GetStatus() == IGame::RUNNING) {
                    game.second->DoUpdate();
                } else if (game.second->GetStatus() == IGame::GAMEOVER) {
                    dead_games_.push_back(game.first);
                }
            }
        }

        // Destroy
        if (dead_games_.size() > 0) {
            for (auto g : dead_games_) {
                GameDestroy(g);
            }
            dead_games_.clear();
        }
    }

    return true;
}

bool GameMgrModule::GameCreate(int id, const string &key) {
    if (games_[id] == nullptr) {
        IGame *game = new Game();
        game->DoInit(id, this);
        game->DoAwake();
        game->DoStart();
        games_[id] = game;
    }
    return true;
}

bool GameMgrModule::GameDestroy(int id) {
    auto iter = games_.find(id);
    if (iter != games_.end()) {
        if (iter->second) {
            iter->second->DoDestroy();
            delete iter->second;
        }
        LOG_INFO("Game destroy<%v>", id);
        games_.erase(iter);
        return true;
    } else {
        LOG_ERROR("Not found this game<%v>", id);
    }
    return false;
}

bool GameMgrModule::DoGamePlayerQuit(const Guid &player) {
    // int id = m_player_manager_->GetPlayerGameplayID(player);
    int id = -1;

    if (id != -1) {
        auto iter = games_.find(id);
        if (iter == games_.end()) {
            // dout << "No this gameplay: " << id << std::endl;
        }
        auto gameplay = games_[id];
        if (gameplay != nullptr) {
            gameplay->DoPlayerQuit(player);
            // Check can destroy this game
            if (gameplay->OnlinePlayerCount() < 1) {
                return GameDestroy(id);
            }
        } else {
            // dout << "This player not in game." << std::endl;
        }
    } else {
        // dout << "This player not in game." << std::endl;
    }
    return false;
}

bool GameMgrModule::SingleGameCreate(int id, const string &key) {
    rpc::ReqGameplayCreate xMsg;
    xMsg.set_id(id);
    xMsg.set_key(key);
    xMsg.set_game_id(pm_->GetAppID());

    // m_pGameServerNet_ServerModule->SendMsgPBToGameplayManager(GameplayManagerRPC::REQ_GAMEPLAY_CREATE, xMsg);
    // m_node_->SendToServer(GameplayManagerRPC::REQ_GAMEPLAY_CREATE, )
    return true;
}

bool GameMgrModule::SingleGameDestroy(int id) { return true; }

void GameMgrModule::OnRecv(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len) {
    rpc::MsgBase xMsg;
    if (!xMsg.ParseFromArray(msg, len)) {
        char szData[MAX_PATH] = {0};
        SQUICK_SPRINTF(szData, MAX_PATH, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msg_id);
        return;
    }

    Guid clientID;
    // clientID.FromString(xMsg.guid());
    // int group_id = m_player_manager_->GetPlayerRoomID(clientID);
    int group_id = -1;
    if (group_id == -1)
        return;

    auto iter = games_.find(group_id);
    if (iter == games_.end()) {
        // dout << "no this group: " << group_id << " msg_id: " << msg_id << std::endl;
        return;
    }

    auto gameplay = iter->second;
    if (gameplay != nullptr) {
        GAME_MGR_RECEIVE_FUNCTOR_PTR &ptr = GetCallback(msg_id, group_id);
        if (ptr != nullptr) {
            GAME_MGR_RECEIVE_FUNCTOR *pFunc = ptr.get();
            pFunc->operator()(clientID, msg_id, xMsg.msg_data());
        } else {
            // dout << "no this callback! msg_id: " << msg_id << std::endl;
        }

    } else {
        // dout << "no this group: " << group_id << " msg_id: " << msg_id << std::endl;
    }
}

} // namespace game::logic