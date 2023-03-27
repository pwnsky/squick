#include "http_module.h"
#include "struct.h"

namespace login::http {
	bool HttpModule::Start() {
		m_http_server_ = pPluginManager->FindModule<IHttpServerModule>();
		m_kernel_ = pPluginManager->FindModule<IKernelModule>();
		m_server_ = pPluginManager->FindModule<server::IServerModule>();
		m_class_ = pPluginManager->FindModule<IClassModule>();
		m_element_ = pPluginManager->FindModule<IElementModule>();
		m_master_ = pPluginManager->FindModule<client::IMasterModule>();
		m_net_client_ = pPluginManager->FindModule<INetClientModule>();
		m_mysql_ = pPluginManager->FindModule<mysql::IMysqlModule>();
		m_redis_ = pPluginManager->FindModule<redis::IRedisModule>();

		return true;
	}


	bool HttpModule::Destory() { return true; }

	bool HttpModule::AfterStart() {
		//dout << "\n加载登录http模块\n";
		m_http_server_->AddRequestHandler("/login", HttpType::SQUICK_HTTP_REQ_POST, this, &HttpModule::OnLogin);
		m_http_server_->AddRequestHandler("/cdn", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::OnGetCDN);

		m_http_server_->AddRequestHandler("/world/list", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::OnWorldList);
		m_http_server_->AddRequestHandler("/world/enter", HttpType::SQUICK_HTTP_REQ_POST, this, &HttpModule::OnWorldEnter);
		m_http_server_->AddNetFilter("/world", this, &HttpModule::OnFilter);
		

		SQUICK_SHARE_PTR<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
		if (xLogicClass) {
			const std::vector<std::string>& strIdList = xLogicClass->GetIDList();
			for (int i = 0; i < strIdList.size(); ++i) {
				const std::string& strId = strIdList[i];

				int web_port = m_element_->GetPropertyInt32(strId, excel::Server::WebPort());
				int nWebServerAppID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
				// webserver only run one instance in each server
				if (pPluginManager->GetAppID() == nWebServerAppID && web_port > 0) {
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

	bool HttpModule::OnLogin(SQUICK_SHARE_PTR<HttpRequest> request) {
		std::string res_str;
		ReqLogin req;
		AckLogin ack;
		ajson::load_from_buff(req, request->body.c_str());
		ajson::string_stream rep_ss;
		do {
			// Verify login
			switch (req.type)
			{
			case LoginType::AccountPasswordLogin: {
				int al = req.account.length();
				int pl = req.password.length();

				if (al < 6 || al > 32 || pl < 6 || pl > 32) {
					ack.code = 1;
					ack.msg = "account or password length is invalid\n";
					break;
				}
				if (m_mysql_->IsHave("account", "")) {
					// 登录
				}
				else {
					// 注册

				}
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
			Guid guid = m_kernel_->CreateGUID(); // just for test
			

			// Account cache to redis
			// ok
			Guid xGUIDKey = m_kernel_->CreateGUID();
			ack.code = 0;
			ack.token = xGUIDKey.ToString();
			mToken[req.account] = xGUIDKey.ToString();
		} while (false);

		ajson::save_to(rep_ss, ack);
		return m_http_server_->ResponseMsg(request, rep_ss.str(), WebStatus::WEB_OK);
	}

	bool HttpModule::OnWorldList(SQUICK_SHARE_PTR<HttpRequest> req) {
		AckWorldList ack;

		MapEx<int, SquickStruct::ServerInfoReport>& xWorldMap = m_master_->GetWorldMap();
		SquickStruct::ServerInfoReport* pWorldData = xWorldMap.FirstNude();
		while (pWorldData) {
			AckWorldList::World world;
			world.id = pWorldData->server_id();
			world.name = pWorldData->server_name();
			world.state = pWorldData->server_state();
			world.count = pWorldData->server_cur_count();
			ack.world.push_back(world);
			pWorldData = xWorldMap.NextNude();
		}

		ajson::string_stream rep_ss;
		ajson::save_to(rep_ss, ack);

		return m_http_server_->ResponseMsg(req, rep_ss.str(), WebStatus::WEB_OK);
	}

	bool HttpModule::OnWorldEnter(SQUICK_SHARE_PTR<HttpRequest> request) {
		std::string strResponse;
		IResponse xResponse;

		std::string user = GetUserID(request);
		ReqWorldEnter req;
		ajson::load_from_buff(req, request->body.c_str());
		if (req.world_id == 0) {
			xResponse.code = IResponse::ResponseType::RES_TYPE_FAILED;

			ajson::string_stream ss;
			ajson::save_to(ss, xResponse);
			strResponse = ss.str();

			return m_http_server_->ResponseMsg(request, strResponse, WebStatus::WEB_OK);
		}
		return m_http_server_->ResponseMsg(request, strResponse, WebStatus::WEB_OK);
	}

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


		std::cout << "OnFilter: " << std::endl;
		std::cout << "url: " << req->url << std::endl;
		std::cout << "path: " << req->path << std::endl;
		std::cout << "type: " << req->type << std::endl;
		std::cout << "body: " << req->body << std::endl;

		std::cout << "params: " << std::endl;

		for (auto item : req->params)
		{
			std::cout << item.first << ":" << item.second << std::endl;
		}

		std::cout << "headers: " << std::endl;

		for (auto item : req->headers)
		{
			std::cout << item.first << ":" << item.second << std::endl;
		}
		return WebStatus::WEB_AUTH;
		
		
	}

	bool HttpModule::OnGetCDN(SQUICK_SHARE_PTR<HttpRequest> req) {

		json repRoot;
		json cdnServerList;

		repRoot["code"] = 0;
		repRoot["msg"] = "";

		SQUICK_SHARE_PTR<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
		if (xLogicClass) {
			const std::vector<std::string>& strIdList = xLogicClass->GetIDList();
			for (int i = 0; i < strIdList.size(); ++i) {
				const std::string& strId = strIdList[i];
				int type = m_element_->GetPropertyInt32(strId, excel::Server::Type());
				if (type != SQUICK_SERVER_TYPES::SQUICK_ST_CDN) {
					continue;
				}
				int web_port = m_element_->GetPropertyInt32(strId, excel::Server::WebPort());
				int rpc_port = m_element_->GetPropertyInt32(strId, excel::Server::Port());
				int server_id = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
				string server_name = m_element_->GetPropertyString(strId, excel::Server::ID());
				string public_ip = m_element_->GetPropertyString(strId, excel::Server::PublicIP());
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
		return m_http_server_->ResponseMsg(req, repRoot.dump(), WebStatus::WEB_OK);
	}

} // namespace login::http


