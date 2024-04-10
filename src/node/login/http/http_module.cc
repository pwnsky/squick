#include "http_module.h"
#include "struct.h"
#include <third_party/common/sha256.h>
#include <third_party/common/base64.hpp>
#define SQUICK_HASH_SALT "7e82e88dfd98952b713c0d20170ce12b"
namespace login::http {
bool HttpModule::Start() {
    m_http_server_ = pm_->FindModule<IHttpServerModule>();
    m_node_ = pm_->FindModule<node::INodeModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    return true;
}

bool HttpModule::Destroy() { return true; }

bool HttpModule::AfterStart() {
    m_http_server_->AddMiddleware(this, &HttpModule::Middleware);

    m_http_server_->AddRequestHandler("/login", HttpType::SQUICK_HTTP_REQ_POST, this, &HttpModule::OnLogin);
    m_http_server_->StartServer(pm_->GetArg("http_port=", 80));

    return true;
}

bool HttpModule::Update() {
    m_http_server_->Update();
    return true;
}


Guid HttpModule::CreatePlayerGUID() {
    int64_t value = 0;
    uint64_t time = SquickGetTimeMS();

    // value = time << 16;
    value = time * 1000000;
    value += player_index ++;

    // if (sequence_ == 0x7FFF)
    if (player_index == 999999) {
        player_index = 0;
    }

    Guid xID;
    xID.nHead64 = (pm_->GetArea() << 32) + pm_->GetAppID();
    xID.nData64 = value;

    return xID;
}

Coroutine<bool>  HttpModule::OnLogin(std::shared_ptr<HttpRequest> request) {
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
            // check from db
        }

        // find min work load proxy
        rpc::NReqMinWorkloadNodeInfo pbreq;
        pbreq.add_type_list(ST_PROXY);
        auto data = co_await m_net_client_->RequestPB(DEFAULT_NODE_MASTER_ID, rpc::NMasterRPC::NREQ_MIN_WORKLOAD_NODE_INFO, pbreq, rpc::NMasterRPC::NACK_MIN_WORKLOAD_NODE_INFO);
        if (data.error) {
            ack.code = IResponse::SERVER_ERROR;
            ack.msg = "Get min workload proxy info from master error, network error\n";
            break;
        }

        uint64_t uid;
        rpc::NAckMinWorkloadNodeInfo pback;
        if (!INetModule::ReceivePB(data.ack_msg_id, data.data, data.length, pback, uid)) {
            ack.code = IResponse::SERVER_ERROR;
            ack.msg = "Get min workload proxy info from master error, pb data is invalid\n";
            break;
        }

        if (pback.list_size() <= 0) {
            ack.code = IResponse::SERVER_ERROR;
            ack.msg = "Get min workload proxy info from master error, no proxy\n";
            break;
        }

        account_id = CreatePlayerGUID().ToString();
        auto proxy_info = pback.list()[0];
        string token = MakeToken(account_id);
        
        ack.code = IResponse::SUCCESS;
        ack.token = token;
        ack.account_id = account_id;
        ack.limit_time = 1209600;
        ack.ip = proxy_info.public_ip();
        ack.port = proxy_info.port();
        ack.ws_port = proxy_info.ws_port();
        ack.key = token;
        ack.signatrue = 123456;
        ack.login_node = pm_->GetAppID();

        time_t login_time = SquickGetTimeS();

        json j;
        j["account"] = req.account;
        j["account_id"] = account_id;
        j["token"] = token;
        j["login_time"] = login_time;
        SetToken(account_id, token);
        
        string cookie = "Session=" + base64_encode(j.dump()) + ";Path=/;Max-Age=1209600";
        m_http_server_->SetHeader(request, "Set-Cookie", cookie.c_str());

    } while (false);
    

    ajson::save_to(rep_ss, ack);
    
    m_http_server_->ResponseMsg(request, rep_ss.str(), WebStatus::WEB_OK);
    co_return;
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
        m_http_server_->ResponseMsg(req, "{\"code\":-1, \"msg\"=\"Forbiden\"}", WebStatus::WEB_AUTH);
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

    auto iter = auth_token_.find(account_id);
    if (iter == auth_token_.end()) {
        return false;
    }

    if (iter->second != user_token) {
        return false;
    }
    return true;
}


string HttpModule::MakeToken(string account_id) {
    string sum = "UserID: " + account_id + std::to_string(SquickGetTimeMS()) + SQUICK_HASH_SALT;
    crypto::SHA256 sha256;
    return sha256(sum);
}

void HttpModule::SetToken(const std::string& account_id, const std::string& user_token) {
    auth_token_[account_id] = user_token;
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

} // namespace login::http
