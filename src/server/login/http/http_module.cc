#include "http_module.h"
#include "struct.h"

namespace login::http {
	bool HttpModule::Start() {
		http_server_module_ = pPluginManager->FindModule<IHttpServerModule>();
		kernel_module_ = pPluginManager->FindModule<IKernelModule>();
		login_server_module_ = pPluginManager->FindModule<server::IServerModule>();
		config_class_module_ = pPluginManager->FindModule<IClassModule>();
		config_element_module_ = pPluginManager->FindModule<IElementModule>();
		client_master_module_ = pPluginManager->FindModule<client::IMasterModule>();
		net_client_module_ = pPluginManager->FindModule<INetClientModule>();

		redis_module_ = pPluginManager->FindModule<redis::IRedisModule>();

		return true;
	}


	bool HttpModule::Destory() { return true; }

	bool HttpModule::AfterStart() {
		//dout << "\n加载登录http模块\n";
		http_server_module_->AddRequestHandler("/login", HttpType::SQUICK_HTTP_REQ_POST, this, &HttpModule::OnLogin);
		http_server_module_->AddRequestHandler("/world", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::OnWorldView);
		http_server_module_->AddRequestHandler("/world", HttpType::SQUICK_HTTP_REQ_CONNECT, this, &HttpModule::OnWorldSelect);
		http_server_module_->AddNetFilter("/world", this, &HttpModule::OnFilter);
		http_server_module_->AddRequestHandler("/cdn", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::OnGetCDN);

		SQUICK_SHARE_PTR<IClass> xLogicClass = config_class_module_->GetElement(excel::Server::ThisName());
		if (xLogicClass) {
			const std::vector<std::string>& strIdList = xLogicClass->GetIDList();
			for (int i = 0; i < strIdList.size(); ++i) {
				const std::string& strId = strIdList[i];

				int web_port = config_element_module_->GetPropertyInt32(strId, excel::Server::WebPort());
				int nWebServerAppID = config_element_module_->GetPropertyInt32(strId, excel::Server::ServerID());
				// webserver only run one instance in each server
				if (pPluginManager->GetAppID() == nWebServerAppID && web_port > 0) {
					http_server_module_->StartServer(web_port);
					break;
				}
			}
		}

		return true;
	}

	bool HttpModule::Update() {
		http_server_module_->Update();
		return true;
	}

	bool HttpModule::OnLogin(SQUICK_SHARE_PTR<HttpRequest> request) {
		std::string res_str;
		ResponseLogin rep;
		RequestLogin req;
		ajson::load_from_buff(req, request->body.c_str());
		ajson::string_stream rep_ss;

		do {
			// Verify login
			switch (req.type)
			{
			case LoginType::AccountPasswordLogin: {

			}break;
			case LoginType::EmailPasswordLogin: {

			}break;
			case LoginType::EmailVerifyCodeLogin: {

			}break;
			case LoginType::PhonePasswordLogin: {

			}break;
			case LoginType::PhoneVerifyCodeLogin: {

			}break;
			case LoginType::WechatLogin: {

			}break;
			case LoginType::QQLogin: {

			}break;
			case LoginType::VisitorLogin: {

			}break;
			case LoginType::TokenLogin: {

			}break;
			default:
				break;
			}


			// Get account guid from mysql
			Guid guid = kernel_module_->CreateGUID(); // just for test
			


			// ok
			Guid xGUIDKey = kernel_module_->CreateGUID();
			rep.code = 0;
			rep.token = xGUIDKey.ToString();
			mToken[req.account] = xGUIDKey.ToString();
		} while (false);

		ajson::save_to(rep_ss, rep);
		return http_server_module_->ResponseMsg(request, rep_ss.str(), WebStatus::WEB_OK);
	}

	bool HttpModule::OnWorldView(SQUICK_SHARE_PTR<HttpRequest> req) {
		std::string strResponse;
		ResponseWorldList xResponsetWorldList;

		MapEx<int, SquickStruct::ServerInfoReport>& xWorldMap = client_master_module_->GetWorldMap();
		SquickStruct::ServerInfoReport* pWorldData = xWorldMap.FirstNude();
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

		return http_server_module_->ResponseMsg(req, strResponse, WebStatus::WEB_OK);
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

			return http_server_module_->ResponseMsg(req, strResponse, WebStatus::WEB_OK);
		}
		return http_server_module_->ResponseMsg(req, strResponse, WebStatus::WEB_OK);
	}

	bool HttpModule::OnCommonQuery(SQUICK_SHARE_PTR<HttpRequest> req) { return http_server_module_->ResponseMsg(req, "OnCommonQuery", WebStatus::WEB_ERROR); }

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

	bool HttpModule::CheckUserJWT(const std::string& user, const std::string& jwt) {
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

	bool HttpModule::OnGetCDN(SQUICK_SHARE_PTR<HttpRequest> req) {

		json repRoot;
		json cdnServerList;

		repRoot["code"] = 0;
		repRoot["msg"] = "";

		SQUICK_SHARE_PTR<IClass> xLogicClass = config_class_module_->GetElement(excel::Server::ThisName());
		if (xLogicClass) {
			const std::vector<std::string>& strIdList = xLogicClass->GetIDList();
			for (int i = 0; i < strIdList.size(); ++i) {
				const std::string& strId = strIdList[i];
				int type = config_element_module_->GetPropertyInt32(strId, excel::Server::Type());
				if (type != SQUICK_SERVER_TYPES::SQUICK_ST_CDN) {
					continue;
				}
				int web_port = config_element_module_->GetPropertyInt32(strId, excel::Server::WebPort());
				int rpc_port = config_element_module_->GetPropertyInt32(strId, excel::Server::Port());
				int server_id = config_element_module_->GetPropertyInt32(strId, excel::Server::ServerID());
				string server_name = config_element_module_->GetPropertyString(strId, excel::Server::ID());
				string public_ip = config_element_module_->GetPropertyString(strId, excel::Server::PublicIP());
				json server;
				server = {
						{"name", server_name},
						{"ip", public_ip},
						{"port", rpc_port},
						{"id", server_id},
						{"type", "cdn"},
						{"http_url", "http://" + public_ip + ":" + to_string(web_port)}
				};

				cdnServerList.push_back(server);

			}

			repRoot["servers"] = cdnServerList;
		}
		return http_server_module_->ResponseMsg(req, repRoot.dump(), WebStatus::WEB_OK);
	}

} // namespace login::http


