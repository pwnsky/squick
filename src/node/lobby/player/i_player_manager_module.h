#pragma once

// #include "player.h"
#include <squick/core/base.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/net/export.h>

#include <node/lobby/node/i_node_module.h>


#include <map>
#include <unordered_map>

class Player;
namespace lobby::player {
typedef std::function<void(const Guid &clientID, const int msg_id, const std::string &data)> GAME_PLAY_RECEIVE_FUNCTOR;
typedef std::shared_ptr<GAME_PLAY_RECEIVE_FUNCTOR> GAME_PLAY_RECEIVE_FUNCTOR_PTR;

class IPlayerManagerModule : public IModule {
  public:
    // virtual Player *GetPlayer(const Guid &clientID) = 0;
    virtual int GetPlayerRoomID(const Guid &clientID) = 0;
    virtual void SetPlayerRoomID(const Guid &clientID, int roomID) = 0;
    virtual int GetPlayerGameplayID(const Guid &clientID) = 0;
    virtual void SetPlayerGameplayID(const Guid &clientID, int gameplayID) = 0;

    template <typename BaseType>
    bool AddReceiveCallBack(const int msg_id, const int id, BaseType *pBase,
                            void (BaseType::*handleReceiver)(const Guid &clientID, const int msg_id, const std::string &data)) {
        GAME_PLAY_RECEIVE_FUNCTOR functor = std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        GAME_PLAY_RECEIVE_FUNCTOR_PTR functorPtr(new GAME_PLAY_RECEIVE_FUNCTOR(functor));

        if (callbacks_.find(msg_id) == callbacks_.end()) // 之前未绑定
        {
            m_net_->RemoveReceiveCallBack(msg_id);
            m_net_->AddReceiveCallBack(msg_id, this, &IPlayerManagerModule::OnRecv);

            std::unordered_map<int, GAME_PLAY_RECEIVE_FUNCTOR_PTR> msg_idMap;
            msg_idMap[id] = functorPtr;
            callbacks_.insert(std::unordered_map<int, std::unordered_map<Guid, GAME_PLAY_RECEIVE_FUNCTOR_PTR>>::value_type(msg_id, msg_idMap));
            return true;
        }

        auto it = callbacks_.find(msg_id);
        it->second[id] = functorPtr;
        return true;
    }

    virtual void OnRecv(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) = 0;

    GAME_PLAY_RECEIVE_FUNCTOR_PTR &GetCallback(int msg_id, string userid) {
        auto &callback = callbacks_[msg_id];
        return callback[userid];
    }

    INetModule *m_net_;
    IClassModule *m_class_;
    IElementModule *m_element_;
    IKernelModule *m_kernel_;
    node::INodeModule *m_node_;
    INetClientModule *m_net_client_;
    IScheduleModule *m_schedule_;
    IDataTailModule *m_data_tail_;
    IEventModule *m_event_;
    ILogModule *m_log_;
    // IRoomModule* m_room_;

  private:
    std::unordered_map<int, std::unordered_map<string, GAME_PLAY_RECEIVE_FUNCTOR_PTR>> callbacks_;
};

} // namespace lobby::player