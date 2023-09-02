#include "http_module.h"
#include "struct.h"
#include <third_party/common/sha256.h>
#include <third_party/common/base64.hpp>
#define SQUICK_HASH_SALT "7e82e88dfd98952b713c0d20170ce12b";
namespace login::http {
bool HttpModule::Start() {
    m_http_server_ = pm_->FindModule<IHttpServerModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_node_ = pm_->FindModule<node::INodeModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_mysql_ = pm_->FindModule<mysql::IMysqlModule>();
    m_redis_ = pm_->FindModule<redis::IRedisModule>();

    return true;
}

bool HttpModule::Destory() { return true; }

bool HttpModule::AfterStart() {
    m_http_server_->AddMiddleware(this, &HttpModule::Middleware);

    m_http_server_->AddRequestHandler("/login", HttpType::SQUICK_HTTP_REQ_POST, this, &HttpModule::OnLogin);
    m_http_server_->AddRequestHandler("/cdn", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::OnGetCDN);

    m_http_server_->AddRequestHandler("/world/list", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::OnWorldList);
    m_http_server_->AddRequestHandler("/world/enter", HttpType::SQUICK_HTTP_REQ_POST, this, &HttpModule::OnWorldEnter);
    //m_http_server_->AddNetFilter("/area/list", this, &HttpModule::OnFilter);
    //m_http_server_->AddNetFilter("/area/enter", this, &HttpModule::OnFilter);

    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            int web_port = m_element_->GetPropertyInt32(strId, excel::Server::WebPort());
            int nWebServerAppID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            // webserver only run one instance in each server
            if (pm_->GetAppID() == nWebServerAppID && web_port > 0) {
                m_http_server_->StartServer(web_port);
                break;
            }
        }
    }

    return true;
}

bool HttpModule::Update() {
    m_http_server_->Update();
    return true;
}

bool HttpModule::OnLogin(std::shared_ptr<HttpRequest> request) {
    std::string res_str;
    ReqLogin req;
    AckLogin ack;
    string account_id;
    ajson::load_from_buff(req, request->body.c_str());
    ajson::string_stream rep_ss;
    do {

        if (req.type == LoginType::AccountPasswordLogin) {
            int al = req.account.length();
            int pl = req.password.length();
#ifdef SQUICK_DEV
            if (al < 1 || al > 32 || pl < 1 || pl > 32) {
                ack.code = IResponse::QEUEST_ERROR;
                ack.msg = "account or password length is invalid\n";
                break;
            }
#else
            if (al < 6 || al > 32 || pl < 6 || pl > 32) {
                ack.code = IResponse::QEUEST_ERROR;
                ack.msg = "account or password length is invalid\n";
                break;
            }
#endif

            if (!m_mysql_->IsHave("account", req.account)) {
                dout << "AccountPasswordLogin 注册账号: account: " << req.account << " " << req.password << std::endl;
                // 注册该账号
                account_id = m_kernel_->CreatePlayerGUID().ToString();
                m_mysql_->RegisterAccount(account_id, req.account, req.password);
            } else {
                // 获取账号id
                account_id = m_mysql_->GetAccountID(mysql::IMysqlModule::AccountType::Account, req.account);
                if (account_id.empty()) {
                    dout << "系统错误, 该用户不存在\n";
                    ack.code = IResponse::SERVER_ERROR;
                    ack.msg = "server error, this player is not exsited!\n";
                }
            }
        } else if (req.type == LoginType::PhonePasswordLogin) {
        }
        dout << "AccountPasswordLogin: account: " << req.account << " " << req.password << "  AccountID: " << account_id << std::endl;
        string token = MakeToken(account_id);


        ack.code = IResponse::SUCCESS;
        ack.token = token;
        ack.account_id = account_id;
        ack.limit_time = 1209600;

        time_t login_time = SquickGetTimeS();

        // 缓存到redis
        m_redis_->HashSet(account_id, "account", req.account);
        m_redis_->HashSet(account_id, "token", token);
        m_redis_->HashSet(account_id, "account_id", account_id);
        m_redis_->HashSet(account_id, "login_limit_time", std::to_string(1209600)); // 14天
        m_redis_->HashSet(account_id, "login_time", std::to_string(login_time));

        json j;
        j["account"] = req.account;
        j["account_id"] = account_id;
        j["token"] = token;
        j["login_time"] = login_time;

        string cookie = "Session=" + base64_encode(j.dump()) + ";Path=/;Max-Age=1209600";
        m_http_server_->SetHeader(request, "Set-Cookie", cookie.c_str());

    } while (false);
    

    ajson::save_to(rep_ss, ack);
    
    return m_http_server_->ResponseMsg(request, rep_ss.str(), WebStatus::WEB_OK);
}

nlohmann::json HttpModule::GetUser(std::shared_ptr<HttpRequest> req) {
    json ret;
    auto it = req->headers.find("Cookie");
    do {
        if (it != req->headers.end()) {
            string& cookie = it->second;
            
            int start = cookie.find("Session=");
            
            if (start < 0) break;
            string value = cookie.substr(start);
            int end = value.find(";");
            if (end < 0) {
                end = value.size();
            }
            string encode_info = value.substr(start + 8, end - start - 8);
            string info = base64_decode(encode_info);
            
            try {
                ret = json::parse(info);
                break;
            }
            catch (exception e) {
                break;
            }
            break;
        }
    } while (false);
    return ret;
}

WebStatus HttpModule::Middleware(std::shared_ptr<HttpRequest> req) {

    // Check auth:
    // Cookie: Session= base64( AES( json_str{'guid' : "xxxx", "token" : "xxxx"} ) );
    m_http_server_->SetHeader(req, "Server", SERVER_NAME);
    dout << "request path: " << req->path << std::endl;
    // 不用授权可访问的白名单
    vector<string> white_list = {
        "/login",
        "/cdn",
    };

    for (auto& w : white_list) {
        if (w == req->path) {
            return WebStatus::WEB_OK;
        }
    }
    string account_id;
    string token;
    auto info = GetUser(req);
    try {
        account_id = info["account_id"];
        token = info["token"];
    }
    catch (exception e) {
        return WebStatus::WEB_AUTH;
    }
    if (CheckAuth(account_id, token)) {
        return WebStatus::WEB_OK;
    }
    return WebStatus::WEB_AUTH;
}

bool HttpModule::CheckAuth(const std::string& account_id, const std::string& user_token) {
    string token;
    if (account_id.empty() || user_token.empty()) {
        return false;
    }
    if (m_redis_->HashGet(account_id, "token", token)) {
        if (!token.empty() && token == user_token) {
            return true;
        }
    }
    return false;
}


string HttpModule::MakeToken(string account_id) {
    string sum = "UserID: " + account_id + std::to_string(SquickGetTimeMS()) + SQUICK_HASH_SALT;
    SHA256 sha256;
    return sha256(sum);
}

bool HttpModule::OnWorldList(std::shared_ptr<HttpRequest> req) {
    AckWorldList ack;
    auto &servers = m_node_->GetServers();

    for (auto &iter : servers) {
        auto &server = iter.second;
        if (server.info->type() == ServerType::ST_WORLD) {
            AckWorldList::World world;
            world.id = server.info->id();
            world.name = server.info->name();
            world.state = server.info->state();
            world.count = 0;
            ack.world.push_back(world);
        }
    }

    ack.code = IResponse::SUCCESS;
    ack.msg = "";
    ajson::string_stream rep_ss;
    ajson::save_to(rep_ss, ack);
    return m_http_server_->ResponseMsg(req, rep_ss.str(), WebStatus::WEB_OK);
}

bool HttpModule::OnWorldEnter(std::shared_ptr<HttpRequest> request) {

    ReqWorldEnter req;
    AckWorldEnter ack;
    auto info = GetUser(request);
    std::string account_id = info["account_id"];
    ajson::load_from_buff(req, request->body.c_str());
    do {

        // 判断world id是否存在
        auto &servers = m_node_->GetServers();
        auto witer = servers.find(req.world_id);
        if (witer == servers.end()) {
            dout << "客户端选择world_id错误: " << req.world_id << std::endl;
            ack.code = IResponse::QEUEST_ERROR;
            break;
        } else if (witer->second.info->type() != ServerType::ST_WORLD) {
            dout << "客户端选择world_id错误: " << req.world_id << std::endl;
            ack.code = IResponse::QEUEST_ERROR;
            break;
        }

        // 获取区服id
        
        // 选择一个workload最小的proxy给客户端

        // find a server
        int min_proxy_id = -1;
        int min_workload = 99999;
        for (auto &iter : servers) {
            auto server = iter.second;
            if (server.info->type() == ServerType::ST_PROXY && server.info->area() == servers[req.world_id].info->area()) {
                dout << "服务: " << server.info->id() << "  area: " << server.info->area() << "workload " << server.info->workload() << "\n";
                if (min_workload > server.info->workload()) {
                    min_proxy_id = iter.first;
                }
            }
        }

        if (min_proxy_id == -1) {
            dout << "没有Proxy可选\n";
            ack.code = IResponse::SERVER_ERROR;
            break;
        }

        auto server = servers[min_proxy_id];
        string key = MakeToken(account_id);
        ack.code = IResponse::SUCCESS;
        ack.ip = server.info->ip();
        ack.port = server.info->port();
        ack.world_id = req.world_id;
        ack.account_id = account_id;
        ack.key = key;
        ack.limit_time = 86400; // 限制一天

        // 缓存到redis
        m_redis_->HashSet(account_id, "enter_time", std::to_string(SquickGetTimeS()));
        m_redis_->HashSet(account_id, "proxy_ip", server.info->ip());
        m_redis_->HashSet(account_id, "proxy_port", std::to_string(server.info->port()));
        m_redis_->HashSet(account_id, "world_id", std::to_string(req.world_id));
        m_redis_->HashSet(account_id, "proxy_key", key);
        m_redis_->HashSet(account_id, "proxy_limit_time", to_string(86400));
        m_redis_->HashSet(account_id, "world_id", to_string(req.world_id));
    } while (false);

    ajson::string_stream rep_ss;
    ajson::save_to(rep_ss, ack);
    return m_http_server_->ResponseMsg(request, rep_ss.str(), WebStatus::WEB_OK);
}




void HttpModule::PrintRequest(std::shared_ptr<HttpRequest> req) {
    std::cout << "OnFilter: " << std::endl;
    std::cout << "url: " << req->url << std::endl;
    std::cout << "path: " << req->path << std::endl;
    std::cout << "type: " << req->type << std::endl;
    std::cout << "body: " << req->body << std::endl;

    std::cout << "params: " << std::endl;

    for (auto item : req->params) {
        std::cout << item.first << ":" << item.second << std::endl;
    }

    std::cout << "headers: " << std::endl;

    for (auto item : req->headers) {
        std::cout << item.first << ":" << item.second << std::endl;
    }
}

bool HttpModule::OnGetCDN(std::shared_ptr<HttpRequest> req) {

    json repRoot;
    json cdnServerList;

    repRoot["code"] = 0;
    repRoot["msg"] = "";

    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];
            int type = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            if (type != ServerType::ST_CDN) {
                continue;
            }
            int web_port = m_element_->GetPropertyInt32(strId, excel::Server::WebPort());
            int rpc_port = m_element_->GetPropertyInt32(strId, excel::Server::Port());
            int server_id = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            string server_name = m_element_->GetPropertyString(strId, excel::Server::ID());
            string public_ip = m_element_->GetPropertyString(strId, excel::Server::PublicIP());
            json server;
            server = {{"name", server_name}, {"ip", public_ip}, {"port", rpc_port},
                      {"id", server_id},     {"type", "cdn"},   {"http_url", "http://" + public_ip + ":" + to_string(web_port)}};
            cdnServerList.push_back(server);
        }
        repRoot["servers"] = cdnServerList;
    }
    return m_http_server_->ResponseMsg(req, repRoot.dump(), WebStatus::WEB_OK);
}

} // namespace login::http
