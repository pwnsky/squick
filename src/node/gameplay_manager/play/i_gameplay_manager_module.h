#pragma once

#include <google/protobuf/dynamic_message.h>
#include <squick/core/base.h>

#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/net/export.h>

// #include "../client/i_db_module.h"
// #include "../lobby/i_player_manager_module.h"
// #include "../lobby/i_room_module.h"
#include "../server/i_server_module.h"

namespace game::play {

typedef std::function<void(const Guid &clientID, const int msg_id, const std::string &data)> GAME_PLAY_RECEIVE_FUNCTOR;
typedef std::shared_ptr<GAME_PLAY_RECEIVE_FUNCTOR> GAME_PLAY_RECEIVE_FUNCTOR_PTR;

class IGameplayManagerModule : public IModule {
  public:
    virtual bool GameplayCreate(int id, const string &key) = 0;
    virtual bool GameplayDestroy(int id) = 0;
    virtual bool GameplayPlayerQuit(const Guid &player) = 0;
    virtual bool SingleGameplayCreate(int id, const string &key) = 0;
    virtual bool SingleGameplayDestroy(int id) = 0;

    template <typename BaseType>
    bool AddReceiveCallBack(const int msg_id, const int id, BaseType *pBase,
                            void (BaseType::*handleReceiver)(const Guid &clientID, const int msg_id, const std::string &data)) {
        GAME_PLAY_RECEIVE_FUNCTOR functor = std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        GAME_PLAY_RECEIVE_FUNCTOR_PTR functorPtr(new GAME_PLAY_RECEIVE_FUNCTOR(functor));

        if (mxReceiveCallBack.find(msg_id) == mxReceiveCallBack.end()) // 之前未绑定
        {
            m_net_->RemoveReceiveCallBack(msg_id);
            m_net_->AddReceiveCallBack(msg_id, this, &IGameplayManagerModule::OnRecv);

            std::map<int, GAME_PLAY_RECEIVE_FUNCTOR_PTR> msg_idMap;
            msg_idMap[id] = functorPtr;
            mxReceiveCallBack.insert(std::map<int, std::map<int, GAME_PLAY_RECEIVE_FUNCTOR_PTR>>::value_type(msg_id, msg_idMap));
            return true;
        }

        auto it = mxReceiveCallBack.find(msg_id);
        it->second[id] = functorPtr;
        return true;
    }

    virtual void OnRecv(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) = 0;

    GAME_PLAY_RECEIVE_FUNCTOR_PTR &GetCallback(int msg_id, int id) {
        auto &group = mxReceiveCallBack[msg_id];
        return group[id];
    }

    INetModule *m_net_;
    IClassModule *m_class_;
    IElementModule *m_element_;
    IKernelModule *m_kernel_;
    ISceneModule *m_scene_;
    server::IServerModule *m_server_;
    client::IDBModule *m_db_;
    INetClientModule *m_net_client_;
    IScheduleModule *m_schedule_;
    IDataTailModule *m_data_tail_;
    IEventModule *m_event_;

    // lobby::IPlayerManagerModule *m_player_manager_;
    // lobby::IRoomModule *m_room_;

  private:
    std::unordered_map<int, std::map<int, GAME_PLAY_RECEIVE_FUNCTOR_PTR>> mxReceiveCallBack;
};
} // namespace game::play
