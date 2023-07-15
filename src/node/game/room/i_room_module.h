#pragma once
#include <map>
#include <squick/core/base.h>
#include <struct/struct.h>
namespace game::room {
using namespace std;
struct Game {
    int id = -1;
    string key;
    string name;
    string ip;
    int port = 0;
    int scene = 0;
    int mode = 0;
};

struct RoomPlyaer {
    Guid guid;
    string name;
    rpc::RoomPlayerStatus status = rpc::RoomPlayerStatus::ROOM_PLAYER_STATUS_NOT_PREPARE;
};

struct Room {
    int id = -1;
    Guid owner;
    string name;
    rpc::RoomStatus status = rpc::RoomStatus::ROOM_PREPARING;
    int nplayers = 0;
    int max_players = 50;
    std::vector<RoomPlyaer> players;
    Game game;
};

class IRoomModule : public IModule {
  public:
    virtual Room *GetRoomByID(int room_id) = 0;
    virtual void BroadcastToPlayers(const uint16_t msg_id, google::protobuf::Message &xMsg, int roomdID) = 0;
    virtual void GamePlayPrepared(int room_id, const string &name, const string &ip, int port) = 0;
    virtual bool RoomQuit(const Guid &player) = 0;

  protected:
  private:
};

} // namespace game::lobby