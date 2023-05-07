#pragma once
#include <list>
#include <squick/plugin/net/i_request.h>
#include <squick/plugin/net/i_response.h>
#include <squick/struct/struct.h>
#include <string>
using namespace std;

enum class LoginType {
    AccountPasswordLogin = 0,
    EmailPasswordLogin = 1,
    EmailVerifyCodeLogin = 2,
    PhonePasswordLogin = 3,
    PhoneVerifyCodeLogin = 4,
    WechatLogin = 5,
    QQLogin = 6,
    VisitorLogin = 7,
    TokenLogin = 8,
};

enum class ClientPlatform {
    Windows = 0,
    Linux = 1,
    Mac = 2,
    Android = 3,
    IOS = 4,
    Web = 5,
};

class AckLogout : public IResponse {
  public:
};

class AckRefreshToken : public IResponse {
  public:
    string token;   // 该token可用于RPC或http的token
    int limit_time; // token过期倒计时
};

class ReqLogin : public IRequest {
  public:
    LoginType type;
    string account;
    string password;
    string token;
    string signature;
    string version;
    ClientPlatform platform;
    string device;
    string extra; // 第三方登录需要
    string email;
    string phone;
    string verify_code;
};

AJSON(ReqLogin, type, account, password, token, signature, version, platform, device, extra, email, phone, verify_code)

class AckLogin : public IResponse {
  public:
    string token;   // 该token可用于RPC或http的token
    string guid;    // 账号Guid
    int limit_time; // token过期倒计时
};

AJSON(AckLogin, code, msg, token, guid, limit_time)

class ReqWorldEnter : public IRequest {
  public:
    int world_id;
};
AJSON(ReqWorldEnter, world_id)

class AckWorldEnter : public IResponse {
  public:
    string guid; // account的guid
    string key;
    string ip;
    int port;
    int world_id;
    int limit_time;
};
AJSON(AckWorldEnter, code, msg, guid, key, ip, port, world_id, limit_time)

class AckWorldList : public IResponse {
  public:
    class World {
      public:
        int id;
        std::string name;
        rpc::ServerState state;
        int count;
    };
    std::list<World> world;
};

AJSON(AckWorldList::World, id, name, state, count)
AJSON(AckWorldList, code, msg, world)