#pragma once
#include <list>
#include <squick/plugin/net/i_request.h>
#include <squick/plugin/net/i_response.h>
#include <squick/struct/struct.h>
#include <string>
using namespace std;
using namespace SquickStruct;
class RequestLogin : public IRequest {
  public:
    LoginType type;
    string account;
    string password;
    string token;
    string signature;
    string version;
    ClientPlatform platform;
    string device;
    string extra;
    string email;
    string phone;
    string verify_code;
};


AJSON(RequestLogin, type, account, password, token, signature, version, platform, device, extra, email, phone, verify_code)

class ResponseLogin : public IResponse {
    public:
    int code;
    string token;
    string guid;
    int limit_time;
};

AJSON(ResponseLogin, code, token, guid, limit_time)

class RequestSelectWorld : public IRequest {
  public:
    int id;
};
AJSON(RequestSelectWorld, id)

class ResponseWorldList : public IResponse {
  public:
    class World {
      public:
        int id;
        std::string name;
        SquickStruct::ServerState state;
        int count;
    };
    std::list<World> world;
};

AJSON(ResponseWorldList::World, id, name, state, count)
AJSON(ResponseWorldList, world, code, message)