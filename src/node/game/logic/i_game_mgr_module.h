#pragma once

#include <google/protobuf/dynamic_message.h>
#include <squick/core/base.h>

#include <squick/plugin/node/export.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

namespace game::logic {

typedef std::function<void(const Guid &clientID, const int msg_id, const std::string &data)> GAME_MGR_RECEIVE_FUNCTOR;
typedef std::shared_ptr<GAME_MGR_RECEIVE_FUNCTOR> GAME_MGR_RECEIVE_FUNCTOR_PTR;

class IGameMgrModule : public IModule {
  public:
    virtual bool GameCreate(int id, const string &key) = 0;
    virtual bool GameDestroy(int id) = 0;
    virtual bool DoGamePlayerQuit(const Guid &player) = 0;
    virtual bool SingleGameCreate(int id, const string &key) = 0;
    virtual bool SingleGameDestroy(int id) = 0;

    template <typename BaseType>
    bool AddReceiveCallBack(const int msg_id, const int id, BaseType *pBase,
                            void (BaseType::*handleReceiver)(const Guid &clientID, const int msg_id, const std::string &data)) {
        GAME_MGR_RECEIVE_FUNCTOR functor = std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        GAME_MGR_RECEIVE_FUNCTOR_PTR functorPtr(new GAME_MGR_RECEIVE_FUNCTOR(functor));

        if (mxReceiveCallBack.find(msg_id) == mxReceiveCallBack.end()) // Just bind once
        {
            m_net_->RemoveReceiveCallBack(msg_id);
            m_net_->AddReceiveCallBack(msg_id, this, &IGameMgrModule::OnRecv);

            std::map<int, GAME_MGR_RECEIVE_FUNCTOR_PTR> msg_idMap;
            msg_idMap[id] = functorPtr;
            mxReceiveCallBack.insert(std::map<int, std::map<int, GAME_MGR_RECEIVE_FUNCTOR_PTR>>::value_type(msg_id, msg_idMap));
            return true;
        }

        auto it = mxReceiveCallBack.find(msg_id);
        it->second[id] = functorPtr;
        return true;
    }

    virtual void OnRecv(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) = 0;

    GAME_MGR_RECEIVE_FUNCTOR_PTR &GetCallback(int msg_id, int id) {
        auto &group = mxReceiveCallBack[msg_id];
        return group[id];
    }

    INetModule *m_net_;
    ILogModule *m_log_;
    IClassModule *m_class_;
    IElementModule *m_element_;
    INodeModule *m_node_;
    INetClientModule *m_net_client_;

  private:
    std::unordered_map<int, std::map<int, GAME_MGR_RECEIVE_FUNCTOR_PTR>> mxReceiveCallBack;
};
} // namespace game::logic
