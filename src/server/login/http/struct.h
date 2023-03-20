#pragma once
#include <list>
#include <squick/plugin/net/i_request.h>
#include <squick/plugin/net/i_response.h>
#include <squick/struct/struct.h>
#include <string>

class RequestLogin : public IRequest {
  public:
    std::string user;
    std::string password;
};
AJSON(RequestLogin, user, password)

class ResponseLogin : public IResponse {
  public:
    std::string jwt;
};
AJSON(ResponseLogin, jwt, code, message)

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