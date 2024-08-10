#include "logic_module.h"
#include "struct.h"
#include <third_party/common/base64.hpp>
#include <third_party/common/sha256.h>

#define SQUICK_HASH_SALT "7e82e88dfd98952b713c0d20170ce12b"
#define WEB_BASE_PATH "/api"


namespace backstage::logic {

bool LogicModule::Start() {
    m_http_server_ = pm_->FindModule<IHttpServerModule>();
    m_node_ = pm_->FindModule<INodeModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_net_ = pm_->FindModule<INetModule>();
    m_log_ = pm_->FindModule<ILogModule>();

    return true;
}

bool LogicModule::Destroy() { return true; }

bool LogicModule::AfterStart() {
    m_http_server_->AddMiddleware(this, &LogicModule::Middleware);
    m_http_server_->AddRequestHandler(WEB_BASE_PATH "/login", HttpType::SQUICK_HTTP_REQ_POST, this, &LogicModule::OnLogin);
    m_http_server_->AddRequestHandler(WEB_BASE_PATH "/all_nodes", HttpType::SQUICK_HTTP_REQ_GET, this, &LogicModule::OnGetAllNodes);
    m_http_server_->AddRequestHandler(WEB_BASE_PATH "/auth_check", HttpType::SQUICK_HTTP_REQ_POST, this, &LogicModule::OnAuthCheck);
    m_http_server_->AddRequestHandler(WEB_BASE_PATH "/auth_check", HttpType::SQUICK_HTTP_REQ_GET, this, &LogicModule::OnAuthCheck);
    m_http_server_->AddRequestHandler(WEB_BASE_PATH "/reload", HttpType::SQUICK_HTTP_REQ_GET, this, &LogicModule::OnReload);
    m_http_server_->AddRequestHandler(WEB_BASE_PATH "/execute_lua", HttpType::SQUICK_HTTP_REQ_POST, this, &LogicModule::OnLuaExecute);
    m_http_server_->StartServer(pm_->GetArg("http_port=", ARG_DEFAULT_HTTP_PORT));

    vector<int> node_types = {rpc::ST_GLOBAL, rpc::ST_DB_PROXY, rpc::ST_PLAYER, rpc::ST_PROXY, rpc::ST_WORLD, rpc::ST_WEB};
    m_node_->AddSubscribeNode(node_types);

    LoadConfig();
    return true;
}

bool LogicModule::LoadConfig() {
    std::string config_path = pm_->GetWorkPath() + "/config/node/backstage.json";
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        LOG_ERROR("The configure file <%v> is not exsist", config_path);
        return false;
    }
    config_file >> web_config_;
    config_file.close();

    try {
        json header = web_config_.at("ResponseHttpHeader");
        for (auto iter = header.begin(); iter != header.end(); ++iter) {
            config_response_header_[iter.key()] = iter.value();
        }
        json white_uri_list = web_config_.at("WhiteUriList");
        for (auto v : white_uri_list) {
            white_uri_list_.insert(v);
        }
    } catch (std::exception err) {
        LOG_ERROR("Get config from json is error <%v>", err.what());
        return false;
    }

    LOG_INFO("The db config file <%v> is loaded ", config_path);
    return true;
}

bool LogicModule::Update() {
    m_http_server_->Update();
    return true;
}

bool LogicModule::OnAuthCheck(std::shared_ptr<HttpRequest> request) {
    IResponse rsp;
    rsp.code = IResponse::SUCCESS;
    rsp.msg = "authed";
    ajson::string_stream rep_ss;
    ajson::save_to(rep_ss, rsp);
    m_http_server_->ResponseMsg(request, rep_ss.str(), WebStatus::WEB_OK);
    return true;
}

Coroutine<bool> LogicModule::OnReload(std::shared_ptr<HttpRequest> request) {

    IResponse rsp;
    rpc::NReqReload pbreq;
    auto data = co_await m_net_client_->RequestPB(DEFAULT_MASTER_ID, rpc::IdNReqReload, pbreq, rpc::IdNAckReload);
    if (data.error) {
        rsp.code = IResponse::SERVER_ERROR;
        rsp.msg = "Server get network error";
        ajson::string_stream rep_ss;
        ajson::save_to(rep_ss, rsp);
        m_http_server_->ResponseMsg(request, rep_ss.str(), WebStatus::WEB_ERROR);
        co_return;
    }

    uint64_t uid;
    rpc::NAckReload pback;
    if (!INetModule::ReceivePB(data.ack_msg_id, data.data, data.length, pback, uid)) {
        rsp.code = IResponse::SERVER_ERROR;
        rsp.msg = "Parse msg is error error";
        ajson::string_stream rep_ss;
        ajson::save_to(rep_ss, rsp);
        m_http_server_->ResponseMsg(request, rep_ss.str(), WebStatus::WEB_ERROR);
        co_return;
    }

    rsp.code = IResponse::SUCCESS;
    rsp.msg = "reloaded";
    ajson::string_stream rep_ss;
    ajson::save_to(rep_ss, rsp);
    m_http_server_->ResponseMsg(request, rep_ss.str(), WebStatus::WEB_OK);
}

Coroutine<bool> LogicModule::OnLogin(std::shared_ptr<HttpRequest> request) {
    std::string res_str;
    BkReqLogin req;
    BkAckLogin ack;
    string account_id;
    ajson::load_from_buff(req, request->body.c_str());
    ajson::string_stream rep_ss;
    do {
        if (req.type == BkLoginType::AccountPasswordLogin) {
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

        account_id = Guid::CreateID().ToString();
        string token = MakeToken(account_id);

        ack.code = IResponse::SUCCESS;
        ack.token = token;
        ack.account_id = account_id;
        ack.limit_time = 1209600;

        // Cache
        auto &info = login_info_[account_id];
        info.account = req.account;
        info.account_id = account_id;
        info.login_time = SquickGetTimeS();

        time_t login_time = SquickGetTimeS();

        json j;
        j["account"] = req.account;
        j["account_id"] = account_id;
        j["token"] = token;
        j["login_time"] = login_time;
        SetToken(account_id, token);

        string cookie = "Session=" + base64_encode(j.dump()) + ";Path=/;Max-Age=1209600;SameSite=None;Secure=False";
        m_http_server_->SetHeader(request, "Set-Cookie", cookie.c_str());

        LOG_INFO("Account %v has logined, account_id<%v> cookie<%v>", req.account, account_id, cookie);

    } while (false);

    ajson::save_to(rep_ss, ack);

    m_http_server_->ResponseMsg(request, rep_ss.str(), WebStatus::WEB_OK);
    co_return;
}

nlohmann::json LogicModule::GetUser(std::shared_ptr<HttpRequest> req) {
    json ret;
    auto it = req->headers.find("Cookie");
    do {
        if (it != req->headers.end()) {
            string &cookie = it->second;

            int start = cookie.find("Session=");

            if (start < 0)
                break;
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
            } catch (exception e) {
                break;
            }
            break;
        }
    } while (false);
    return ret;
}

WebStatus LogicModule::Middleware(std::shared_ptr<HttpRequest> req) {
    // Check auth:
    // Cookie: Session= base64( AES( json_str{'guid' : "xxxx", "token" : "xxxx"} ) );
    for (auto iter : config_response_header_) {
        m_http_server_->SetHeader(req, iter.first, iter.second);
    }

    m_http_server_->SetHeader(req, "Server", SERVER_NAME);

    if (req->type == SQUICK_HTTP_REQ_OPTIONS) {
        m_http_server_->ResponseMsg(req, "", WebStatus::WEB_OK);
        return WebStatus::WEB_RETURN;
    }

    // check uri
    if (white_uri_list_.find(req->path) != white_uri_list_.end()) {
        return WebStatus::WEB_IGNORE;
    }

    string account_id;
    string token;
    auto info = GetUser(req);
    try {
        account_id = info["account_id"];
        token = info["token"];
    } catch (exception e) {
        return WebStatus::WEB_AUTH;
    }
    if (CheckAuth(account_id, token)) {
        return WebStatus::WEB_IGNORE;
    }
    return WebStatus::WEB_AUTH;
}

bool LogicModule::CheckAuth(const std::string &account_id, const std::string &user_token) {
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

string LogicModule::MakeToken(string account_id) {
    string sum = "UserID: " + account_id + std::to_string(SquickGetTimeMS()) + SQUICK_HASH_SALT;
    crypto::SHA256 sha256;
    return sha256(sum);
}

void LogicModule::SetToken(const std::string &account_id, const std::string &user_token) { auth_token_[account_id] = user_token; }

util::JsonPrintOptions LogicModule::GetDefaultPb2JsonOptions() {
    google::protobuf::util::JsonPrintOptions options;
    options.preserve_proto_field_names = true;
    options.always_print_primitive_fields = true;
    options.always_print_enums_as_ints = true;
    options.add_whitespace = true;
    return options;
}


Coroutine<bool> LogicModule::OnGetAllNodes(std::shared_ptr<HttpRequest> request) {
    json rsp;

    rpc::NReqAllNodesInfo pbreq;
    auto data = co_await m_net_client_->RequestPB(DEFAULT_MASTER_ID, rpc::IdNReqAllNodesInfo, pbreq, rpc::IdNAckAllNodesInfo);
    if (data.error) {
        rsp["code"] = IResponse::SERVER_ERROR;
        rsp["msg"] = "Server get network error\n";
        m_http_server_->ResponseMsg(request, rsp.dump(), WebStatus::WEB_ERROR);
        co_return;
    }

    uint64_t uid;
    rpc::NAckAllNodesInfo pback;
    if (!INetModule::ReceivePB(data.ack_msg_id, data.data, data.length, pback, uid)) {
        rsp["code"] = IResponse::SERVER_ERROR;
        rsp["msg"] = "Parse msg is error error\n";
        m_http_server_->ResponseMsg(request, rsp.dump(), WebStatus::WEB_ERROR);
        co_return;
    }

	// https://protobuf.dev/reference/cpp/api-docs/google.protobuf.util.json_util/

	/*
	// json-string 转 protobuf message
	util::Status google::protobuf::util::JsonStringToMessage(const std::string content, google::protobuf::Message* message);
	// protobuf 转 json-string
	util::Status google::protobuf::util::MessageToJsonString(const google::protobuf::Message& message, std::string* str );
	*/
	std::string json_out;
	google::protobuf::util::Status status = google::protobuf::util::MessageToJsonString(pback, &json_out, GetDefaultPb2JsonOptions());
	LOG_DEBUG("Protobuf to json: %v", json_out);

    nlohmann::json node_list = nlohmann::json::array();
    for (auto &sd : pback.node_list()) {
        json n;
        n["area"] = sd.area();
        n["type"] = sd.type();
        n["id"] = sd.id();
        n["name"] = sd.name();
        n["ip"] = sd.ip();
        n["public_ip"] = sd.public_ip();
        n["port"] = sd.port();
        n["cpu_count"] = sd.cpu_count();
        n["status"] = sd.state();
        n["workload"] = sd.workload();
        n["max_online"] = sd.max_online();
        n["update_time"] = sd.update_time();
        n["ws_port"] = sd.ws_port();
        n["http_port"] = sd.http_port();
        n["https_port"] = sd.https_port();
        n["connections"] = sd.connections();
        n["net_client_connections"] = sd.net_client_connections();
        node_list.push_back(n);
    }
    rsp["node_list"] = node_list;
    rsp["code"] = IResponse::SUCCESS;
    m_http_server_->ResponseMsg(request, rsp.dump(), WebStatus::WEB_OK);
    co_return;
}

Coroutine<bool> LogicModule::OnLuaExecute(std::shared_ptr<HttpRequest> request) {
    
    rpc::NReqExecuteLua pbreq;
    rpc::NAckExecuteLua pback;
    util::Status status = util::JsonStringToMessage(request->body, &pbreq);
    if (!status.ok())
    {
    }

    auto data = co_await m_net_client_->RequestPB(pbreq.node_id(), rpc::IdNReqExecuteLua, pbreq, rpc::IdNAckExecuteLua);
    if (data.error) {
        co_return;
    }

    uint64_t uid;
    if (!INetModule::ReceivePB(data.ack_msg_id, data.data, data.length, pback, uid)) {
        co_return;
    }

    std::string json_out;
    status = util::MessageToJsonString(pbreq, &json_out, GetDefaultPb2JsonOptions());
    LOG_DEBUG("OnLuaExecute Protobuf to json: %v", json_out);

    m_http_server_->ResponseMsg(request, json_out, WebStatus::WEB_OK);
}

} // namespace backstage::logic
