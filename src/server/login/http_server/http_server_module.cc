

#include "request_login.h"
#include "response_login.h"
#include "response_world_list.h"
#include "request_select_world.h"
#include "http_server_module.h"

bool LoginNet_HttpServerModule::Start()
{
	m_pHttpNetModule = pPluginManager->FindModule<IHttpServerModule>();
	m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
	m_pLoginServerModule = pPluginManager->FindModule<ILoginNet_ServerModule>();
	m_pLogicClassModule = pPluginManager->FindModule<IClassModule>();
	m_pElementModule = pPluginManager->FindModule<IElementModule>();
	m_pLoginToMasterModule = pPluginManager->FindModule<ILoginToMasterModule>();
	m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
	
	return true;
}
bool LoginNet_HttpServerModule::Destory()
{
	return true;
}

bool LoginNet_HttpServerModule::AfterStart()
{
	m_pHttpNetModule->AddRequestHandler("/login", HttpType::SQUICK_HTTP_REQ_POST, this, &LoginNet_HttpServerModule::OnLogin);
	m_pHttpNetModule->AddRequestHandler("/world", HttpType::SQUICK_HTTP_REQ_GET, this, &LoginNet_HttpServerModule::OnWorldView);
	m_pHttpNetModule->AddRequestHandler("/world", HttpType::SQUICK_HTTP_REQ_CONNECT, this, &LoginNet_HttpServerModule::OnWorldSelect);

	m_pHttpNetModule->AddNetFilter("/world", this, &LoginNet_HttpServerModule::OnFilter);

	SQUICK_SHARE_PTR<IClass> xLogicClass = m_pLogicClassModule->GetElement(excel::Server::ThisName());
	if (xLogicClass)
	{
		const std::vector<std::string>& strIdList = xLogicClass->GetIDList();
		for (int i = 0; i < strIdList.size(); ++i)
		{
			const std::string& strId = strIdList[i];

			int nJsonPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::WebPort());
			int nWebServerAppID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());

			//webserver only run one instance in each server
			if (pPluginManager->GetAppID() == nWebServerAppID && nJsonPort > 0)
			{
				m_pHttpNetModule->StartServer(nJsonPort);

				break;
			}
		}
	}

	return true;
}

bool LoginNet_HttpServerModule::Update()
{
    //m_pHttpNetModule->Update();
	return true;
}

bool LoginNet_HttpServerModule::OnLogin(SQUICK_SHARE_PTR<HttpRequest> req)
{
	std::string strResponse;
	ResponseLogin xResponsetLogin;

	RequestLogin xRequestLogin;
	ajson::load_from_buff(xRequestLogin, req->body.c_str());
	if (xRequestLogin.user.empty()
		|| xRequestLogin.password.empty())
	{
		xResponsetLogin.code = IResponse::ResponseType::RES_TYPE_FAILED;
		xResponsetLogin.jwt = "";

		ajson::string_stream ss;
		ajson::save_to(ss, xResponsetLogin);
		strResponse = ss.str();
	}
	else
	{
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

bool LoginNet_HttpServerModule::OnWorldView(SQUICK_SHARE_PTR<HttpRequest> req)
{
	std::string strResponse;
	ResponseWorldList xResponsetWorldList;

	MapEx<int, SquickStruct::ServerInfoReport>& xWorldMap = m_pLoginToMasterModule->GetWorldMap();
	SquickStruct::ServerInfoReport* pWorldData = xWorldMap.FirstNude();
	while (pWorldData)
	{
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

bool LoginNet_HttpServerModule::OnWorldSelect(SQUICK_SHARE_PTR<HttpRequest> req)
{
	std::string strResponse;
	IResponse xResponse;

	std::string user = GetUserID(req);

	RequestSelectWorld xRequestSelectWorld;
	ajson::load_from_buff(xRequestSelectWorld, req->body.c_str());
	if (xRequestSelectWorld.id == 0)
	{
		xResponse.code = IResponse::ResponseType::RES_TYPE_FAILED;

		ajson::string_stream ss;
		ajson::save_to(ss, xResponse);
		strResponse = ss.str();

		return m_pHttpNetModule->ResponseMsg(req, strResponse, WebStatus::WEB_OK);
	}

	//SquickStruct::ReqConnectWorld xData;
	//xData.set_world_id(xRequestSelectWorld.id);
	//xData.set_login_id(pPluginManager->GetAppID());
	//xData.mutable_sender()->CopyFrom(INetModule::StructToProtobuf(Guid()));
	//xData.set_account(user);

	//m_pNetClientModule->SendSuitByPB(SQUICK_SERVER_TYPES::SQUICK_ST_MASTER, user, SquickStruct::ServerRPC::REQ_CONNECT_WORLD, xData);
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

bool LoginNet_HttpServerModule::OnCommonQuery(SQUICK_SHARE_PTR<HttpRequest> req)
{
	return m_pHttpNetModule->ResponseMsg(req, "OnCommonQuery", WebStatus::WEB_ERROR);
}

std::string LoginNet_HttpServerModule::GetUserID(SQUICK_SHARE_PTR<HttpRequest> req)
{
	auto it = req->headers.find("user");
	if (it != req->headers.end())
	{
		return it->second;
	}

	return "";
}

std::string LoginNet_HttpServerModule::GetUserJWT(SQUICK_SHARE_PTR<HttpRequest> req)
{
	auto it = req->headers.find("jwt");
	if (it != req->headers.end())
	{
		return it->second;
	}

	return "";
}

bool LoginNet_HttpServerModule::CheckUserJWT(const std::string & user, const std::string & jwt)
{
	auto it = mToken.find(user);
	if (it != mToken.end())
	{
		return (it->second == jwt);
	}

	return false;
}

WebStatus LoginNet_HttpServerModule::OnFilter(SQUICK_SHARE_PTR<HttpRequest> req)
{
	std::string user = GetUserID(req);
	std::string jwt = GetUserJWT(req);

	bool bRet = CheckUserJWT(user, jwt);
	if (bRet)
	{
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
