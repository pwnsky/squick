

#include "gameplay_manager_module.h"
#include <server/db_proxy/logic/common_redis_module.h>

namespace game::play {

bool GameplayManagerModule::Start() {
    m_pElementModule = pPluginManager->FindModule<IElementModule>();
    m_pClassModule = pPluginManager->FindModule<IClassModule>();
    m_pNetModule = pPluginManager->FindModule<INetModule>();
    m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
    m_pGameToDBModule = pPluginManager->FindModule<IGameServerToDBModule>();

    m_pGameServerNet_ServerModule = pPluginManager->FindModule<IGameServerNet_ServerModule>();
    m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
    m_pScheduleModule = pPluginManager->FindModule<IScheduleModule>();
    m_pDataTailModule = pPluginManager->FindModule<IDataTailModule>();
    m_pSceneModule = pPluginManager->FindModule<ISceneModule>();
    m_pEventModule = pPluginManager->FindModule<IEventModule>();

    m_pRoomModule = pPluginManager->FindModule<player::IRoomModule>();
    m_pPlayerManagerModule = pPluginManager->FindModule<player::IPlayerManagerModule>();
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
    dout << "Gameplay销毁\n";
    auto iter = m_gameplay.find(id);
    if (iter != m_gameplay.end()) {
        if (iter->second) {
            iter->second->DoDestroy();
            delete iter->second;
        }
        m_gameplay.erase(iter);
        return true;
    } else {
        dout << "Gameplay没有找到\n";
    }
    return false;
}

bool GameplayManagerModule::GameplayPlayerQuit(const Guid &player) {
    dout << "玩家: " << player.ToString() << " quit \n";
    int id = m_pPlayerManagerModule->GetPlayerGameplayID(player);
    if (id != -1) {
        auto gameplay = m_gameplay[id];
        if (gameplay != nullptr) {
            gameplay->DoPlayerQuit(player);
            // 检查是否可以销毁该gameplay
            if (gameplay->OnlinePlayerCount() < 1) {
                GameplayDestroy(id);
            }
        } else {
            dout << "该玩家没有在游戏对局中" << std::endl;
        }
    } else {
        dout << "该玩家没有在游戏对局中" << std::endl;
    }
    return true;
}

void GameplayManagerModule::OnRecv(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    SquickStruct::MsgBase xMsg;
    if (!xMsg.ParseFromArray(msg, len)) {
        char szData[MAX_PATH] = {0};
        NFSPRINTF(szData, MAX_PATH, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msgID);
        return;
    }

    Guid clientID = m_pNetModule->ProtobufToStruct(xMsg.player_id());
    int group_id = m_pPlayerManagerModule->GetPlayerRoomID(clientID);

    auto gameplay = m_gameplay[group_id];
    if (gameplay != nullptr) {
        GAME_PLAY_RECEIVE_FUNCTOR_PTR &ptr = GetCallback(msgID, group_id);
        if (ptr != nullptr) {
            GAME_PLAY_RECEIVE_FUNCTOR *pFunc = ptr.get();
            pFunc->operator()(clientID, msgID, xMsg.msg_data());
        } else {
            dout << "不存在该 callback! msgID: " << msgID << std::endl;
        }

    } else {
        dout << "不存在该 group: " << group_id << " msgID: " << msgID << std::endl;
        // 不存在该group，可能已销毁
    }
}

} // namespace game::play