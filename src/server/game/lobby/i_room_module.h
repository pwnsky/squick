#pragma once
#include <map>
#include <squick/core/base.h>
#include <squick/struct/struct.h>
namespace game::player {
struct Room {};
class IRoomModule : public IModule {
  public:
    virtual rpc::RoomDetails *GetRoomByID(int room_id) = 0;
    virtual void BroadcastToPlyaers(const uint16_t msg_id, google::protobuf::Message &xMsg, int roomdID) = 0;
    virtual void GamePlayPrepared(int room_id, const string &name, const string &ip, int port) = 0;
    virtual bool RoomQuit(const Guid &clientID) = 0;

  protected:
  private:
};

} // namespace game::player