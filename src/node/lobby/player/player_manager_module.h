#pragma once

#include <squick/core/consistent_hash.h>
// #include <squick/plugin/no_sql/export.h>

#include "i_player_manager_module.h"

#include "player.h"
#include <unordered_map>

namespace lobby::player {
class PlayerManagerModule : public IPlayerManagerModule {
  public:
    PlayerManagerModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = true;
    }

    virtual ~PlayerManagerModule(){};

    virtual bool Start();
    virtual bool Destory();
    virtual bool ReadyUpdate();
    virtual bool Update();
    virtual bool AfterStart();

    // 发送数据到客户端
    virtual void OnSendToClient(const uint16_t msg_id, google::protobuf::Message &xMsg, const Guid &client_id);
    // virtual Player *GetPlayer(const Guid &clientID) override;
    virtual int GetPlayerRoomID(const Guid &clientID) override;
    virtual void SetPlayerRoomID(const Guid &clientID, int roomID) override;
    virtual int GetPlayerGameplayID(const Guid &clientID) override;
    virtual void SetPlayerGameplayID(const Guid &clientID, int gameplayID) override;

    virtual void OnRecv(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) override;

  protected:
    void OnReqPlayerEnter(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqPlayerLeave(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqPlayerReconnect(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqPlayerData(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqPlayerEquipment(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    void OnAckPlayerDataLoad(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    int OnPlayerObjectEvent(const Guid &self, const std::string &className, const CLASS_OBJECT_EVENT classEvent, const DataList &var);

  private:
    void LoadDataFromDb(const Guid &self);
    void SaveDataToDb(const Guid &self);

    int SaveDataOnTime(const Guid &self, const std::string &name, const float fIntervalTime, const int count);
    inline void UpdateRemoveOfflinePlayers(time_t now_time);

  private:
    // 待优化为 unordered_map , 查找时间复杂度为 O(1)
    // std::map<Guid, rpc::PlayerData> mxObjectDataCache;
    std::unordered_map<string, Player *> players_; // 所有玩家
    struct Offline {
        enum Type {
            LobbyOffline,
            PlayingOffline,
        };
        Type type = LobbyOffline;
        time_t time = 0;
    };
    std::unordered_map<string, Offline> offline_players_;

  private:
    // play::IGameplayManagerModule *m_gameplay_manager_;
};

} // namespace lobby::player