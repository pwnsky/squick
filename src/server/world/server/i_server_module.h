#include <squick/core/i_module.h>
#include <squick/core/map_ex.h>
// #include "squick/base/login_net_server.h"
#include <squick/core/guid.h>

class IWorldNet_ServerModule : public IModule {
  public:
    class IPlayerWorldCache {};

    class PlayerData {
      public:
        PlayerData(const Guid id) {
            self = id;
            gameID = 0;
            gateID = 0;
            bp = 0;

            onLine = false;
        }

        virtual ~PlayerData() {}

        void OnLine(const int gameSvrID, const int gateSvrID) {
            gameID = gameSvrID;
            gateID = gateSvrID;
            onLine = true;
        }

        void OffLine() {
            gameID = 0;
            gateID = 0;
            onLine = false;
        }

        bool onLine;
        int gameID = 0;
        int gateID = 0;
        Guid self;
        Guid team;
        Guid clan;
        int bp = 0;
        std::string name;
        std::string headIcon;

        int level = 0;
        int gold = 0;
        int diamond = 0;

        MapEx<int, IPlayerWorldCache> playerWorldCache;
    };

    virtual bool IsPrimaryWorldServer() = 0;
    virtual int GetWorldAreaID() = 0;

    virtual void OnServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) = 0;

    virtual bool SendMsgToGame(const int gameID, const int msg_id, const std::string &xData) = 0;
    virtual bool SendMsgToGame(const int gameID, const int msg_id, const google::protobuf::Message &xData) = 0;

    virtual bool SendMsgToGamePlayer(const Guid nPlayer, const int msg_id, const std::string &xData) = 0;
    virtual bool SendMsgToGamePlayer(const Guid nPlayer, const int msg_id, const google::protobuf::Message &xData) = 0;
    virtual bool SendMsgToGamePlayer(const DataList &argObjectVar, const int msg_id, google::protobuf::Message &xData) = 0;

    virtual std::shared_ptr<ServerData> GetSuitProxyToEnter() = 0;
    virtual std::shared_ptr<ServerData> GetSuitGameToEnter(const int arg) = 0;
    virtual const std::vector<Guid> &GetOnlinePlayers() = 0;

    virtual std::shared_ptr<PlayerData> GetPlayerData(const Guid &id) = 0;

    template <typename BaseType> bool AddCallBackForPlayerOnline(BaseType *pBase, void (BaseType::*handleReceiver)(const Guid self)) {
        auto functor = std::bind(handleReceiver, pBase, std::placeholders::_1);
        std::shared_ptr<std::function<void(const Guid)>> cb(new std::function<void(const Guid)>(functor));
        return AddOnLineReceiveCallBack(cb);
    }
    template <typename BaseType> bool AddCallBackForPlayerOffline(BaseType *pBase, void (BaseType::*handleReceiver)(const Guid self)) {
        auto functor = std::bind(handleReceiver, pBase, std::placeholders::_1);
        std::shared_ptr<std::function<void(const Guid)>> cb(new std::function<void(const Guid)>(functor));
        return AddOffLineReceiveCallBack(cb);
    }

  protected:
    virtual bool AddOnLineReceiveCallBack(std::shared_ptr<std::function<void(const Guid)>> cb) = 0;
    virtual bool AddOffLineReceiveCallBack(std::shared_ptr<std::function<void(const Guid)>> cb) = 0;
};
