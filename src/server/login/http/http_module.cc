#include "http_module.h"
#include "struct.h"

namespace login::http {
bool HttpModule::Start() {
    m_pHttpNetModule = pPluginManager->FindModule<IHttpServerModule>();
    m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
    m_pLoginServerModule = pPluginManager->FindModule<server::IServerModule>();
    m_pLogicClassModule = pPluginManager->FindModule<IClassModule>();
    m_pElementModule = pPluginManager->FindModule<IElementModule>();
    m_pLoginToMasterModule = pPluginManager->FindModule<client::IMasterModule>();
    m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();

    return true;
}
bool HttpModule::Destory() { return true; }

bool HttpModule::AfterStart() {
    m_pHttpNetModule->AddRequestHandler("/login", HttpType::SQUICK_HTTP_REQ_POST, this, &HttpModule::OnLogin);
    m_pHttpNetModule->AddRequestHandler("/world", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::OnWorldView);
    m_pHttpNetModule->AddRequestHandler("/world", HttpType::SQUICK_HTTP_REQ_CONNECT, this, &HttpModule::OnWorldSelect);
    m_pHttpNetModule->AddNetFilter("/world", this, &HttpModule::OnFilter);

    SQUICK_SHARE_PTR<IClass> xLogicClass = m_pLogicClassModule->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            int nJsonPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::WebPort());
            int nWebServerAppID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());

            // webserver only run one instance in each server
            if (pPluginManager->GetAppID() == nWebServerAppID && nJsonPort > 0) {
                m_pHttpNetModule->StartServer(nJsonPort);

                break;
            }
        }
    }

    return true;
}

bool HttpModule::Update() {
    // m_pHttpNetModule->Update();
    return true;
}

bool HttpModule::OnLogin(SQUICK_SHARE_PTR<HttpRequest> req) {
    std::string strResponse;
    ResponseLogin xResponsetLogin;

    RequestLogin xRequestLogin;
    ajson::load_from_buff(xRequestLogin, req->body.c_str());
    if (xRequestLogin.user.empty() || xRequestLogin.password.empty()) {
        xResponsetLogin.code = IResponse::ResponseType::RES_TYPE_FAILED;
        xResponsetLogin.jwt = "";

        ajson::string_stream ss;
        ajson::save_to(ss, xResponsetLogin);
        strResponse = ss.str();
    } else {
        Guid xGUIDKey = m_pKernelModule->CreateGUID();
        xResponsetLogin.code = IResponse::ResponseType::RES_TYPE_SUCCESS;
        xResponsetLogin.jwt = xGUIDKey.ToString();
        mToken[xRequestLogin.user] = xGUIDKey.ToString();

        ajson::string_stream ss;
        ajson::save_to(ss, xResponsetLogin);
        strResponse = ss.str();
    }

    return m_pHttpNetModule->ResponseMsg(req, strResponse, WebStatus::WEB_OK);
}

bool HttpModule::OnWorldView(SQUICK_SHARE_PTR<HttpRequest> req) {
    std::string strResponse;
    ResponseWorldList xResponsetWorldList;

    MapEx<int, SquickStruct::ServerInfoReport> &xWorldMap = m_pLoginToMasterModule->GetWorldMap();
    SquickStruct::ServerInfoReport *pWorldData = xWorldMap.FirstNude();
    while (pWorldData) {
        ResponseWorldList::World xWorld;

        xWorld.id = pWorldData->server_id();
        xWorld.name = pWorldData->server_name();
        xWorld.state = pWorldData->server_state();
        xWorld.count = pWorldData->server_cur_count();

        xResponsetWorldList.world.push_back(xWorld);

        pWorldData = xWorldMap.NextNude();
    }

    ajson::string_stream ss;
    ajson::save_to(ss, xResponsetWorldList);
    strResponse = ss.str();

    return m_pHttpNetModule->ResponseMsg(req, strResponse, WebStatus::WEB_OK);
}

bool HttpModule::OnWorldSelect(SQUICK_SHARE_PTR<HttpRequest> req) {
    std::string strResponse;
    IResponse xResponse;

    std::string user = GetUserID(req);

    RequestSelectWorld xRequestSelectWorld;
    ajson::load_from_buff(xRequestSelectWorld, req->body.c_str());
    if (xRequestSelectWorld.id == 0) {
        xResponse.code = IResponse::ResponseType::RES_TYPE_FAILED;

        ajson::string_stream ss;
        ajson::save_to(ss, xResponse);
        strResponse = ss.str();

        return m_pHttpNetModule->ResponseMsg(req, strResponse, WebStatus::WEB_OK);
    }

    // SquickStruct::ReqConnectWorld xData;
    // xData.set_world_id(xRequestSelectWorld.id);
    // xData.set_login_id(pPluginManager->GetAppID());
    // xData.mutable_sender()->CopyFrom(INetModule::StructToProtobuf(Guid()));
    // xData.set_account(user);

    // m_pNetClientModule->SendSuitByPB(SQUICK_SERVER_TYPES::SQUICK_ST_MASTER, user, SquickStruct::ServerRPC::REQ_CONNECT_WORLD, xData);
    /*
    while (true)
    {
            YieldCo();

            if ()
            {
                    break;
            }
    }
    */

    return m_pHttpNetModule->ResponseMsg(req, strResponse, WebStatus::WEB_OK);
}

bool HttpModule::OnCommonQuery(SQUICK_SHARE_PTR<HttpRequest> req) { return m_pHttpNetModule->ResponseMsg(req, "OnCommonQuery", WebStatus::WEB_ERROR); }

std::string HttpModule::GetUserID(SQUICK_SHARE_PTR<HttpRequest> req) {
    auto it = req->headers.find("user");
    if (it != req->headers.end()) {
        return it->second;
    }

    return "";
}

std::string HttpModule::GetUserJWT(SQUICK_SHARE_PTR<HttpRequest> req) {
    auto it = req->headers.find("jwt");
    if (it != req->headers.end()) {
        return it->second;
    }

    return "";
}

bool HttpModule::CheckUserJWT(const std::string &user, const std::string &jwt) {
    auto it = mToken.find(user);
    if (it != mToken.end()) {
        return (it->second == jwt);
    }

    return false;
}

WebStatus HttpModule::OnFilter(SQUICK_SHARE_PTR<HttpRequest> req) {
    std::string user = GetUserID(req);
    std::string jwt = GetUserJWT(req);

    bool bRet = CheckUserJWT(user, jwt);
    if (bRet) {
        return WebStatus::WEB_OK;
    }

    return WebStatus::WEB_AUTH;
    /*
    std::cout << "OnFilter: " << std::endl;

    std::cout << "url: " << req.url << std::endl;
    std::cout << "path: " << req.path << std::endl;
    std::cout << "type: " << req.type << std::endl;
    std::cout << "body: " << req.body << std::endl;

    std::cout << "params: " << std::endl;

    for (auto item : req.params)
    {
            std::cout << item.first << ":" << item.second << std::endl;
    }

    std::cout << "headers: " << std::endl;

    for (auto item : req.headers)
    {
            std::cout << item.first << ":" << item.second << std::endl;
    }

    return WebStatus::WEB_OK;
    */
}

} // namespace login::http