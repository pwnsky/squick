#include "http_module.h"
#include "struct.h"

namespace login::http {
	bool HttpModule::Start() {
		m_http_server_ = pm_->FindModule<IHttpServerModule>();
		m_kernel_ = pm_->FindModule<IKernelModule>();
		m_server_ = pm_->FindModule<server::IServerModule>();
		m_class_ = pm_->FindModule<IClassModule>();
		m_element_ = pm_->FindModule<IElementModule>();
		m_master_ = pm_->FindModule<client::IMasterModule>();
		m_net_client_ = pm_->FindModule<INetClientModule>();
		m_mysql_ = pm_->FindModule<mysql::IMysqlModule>();
		m_redis_ = pm_->FindModule<redis::IRedisModule>();

		return true;
	}


	bool HttpModule::Destory() { return true; }

	bool HttpModule::AfterStart() {
		//dout << "\n加载登录http模块\n";
		m_http_server_->AddRequestHandler("/login", HttpType::SQUICK_HTTP_REQ_POST, this, &HttpModule::OnLogin);
		m_http_server_->AddRequestHandler("/cdn", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::OnGetCDN);

		m_http_server_->AddRequestHandler("/world/list", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::OnWorldList);
		m_http_server_->AddRequestHandler("/world/enter", HttpType::SQUICK_HTTP_REQ_POST, this, &HttpModule::OnWorldEnter);
		m_http_server_->AddNetFilter("/world/list", this, &HttpModule::OnFilter);
		m_http_server_->AddNetFilter("/world/enter", this, &HttpModule::OnFilter);

		std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
		if (xLogicClass) {
			const std::vector<std::string>& strIdList = xLogicClass->GetIDList();
			for (int i = 0; i < strIdList.size(); ++i) {
				const std::string& strId = strIdList[i];

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
		dout << "请求登录\n";
		std::string res_str;
		ReqLogin req;
		AckLogin ack;
		Guid guid;
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
					guid = m_kernel_->CreateGUID();
					m_mysql_->RegisterAccount(guid.ToString(), req.account, req.password);
				}
				else {
					// 获取账号guid
					guid = m_mysql_->GetGuid(mysql::IMysqlModule::AccountType::Account, req.account);
					if (guid == Guid()) {
						dout << "系统错误, 该用户不存在\n";
						ack.code = IResponse::SERVER_ERROR;
						ack.msg = "server error, this player is not exsited!\n";
					}
				}
			}
			else if (req.type == LoginType::PhonePasswordLogin) {

			}

			dout << "AccountPasswordLogin 登录账号: account: " << req.account << " " << req.password << std::endl;
			dout << "Guid: " << guid.ToString() << std::endl;

			// Account cache to redis
			// ok
			Guid token = m_kernel_->CreateGUID();
			ack.code = IResponse::SUCCESS;
			ack.token = token.ToString();
			ack.guid = guid.ToString();
			ack.limit_time = 1209600; // 14天
			tokens_[guid.ToString()] = token.ToString();

			// 缓存到redis


		} while (false);

		ajson::save_to(rep_ss, ack);
		return m_http_server_->ResponseMsg(request, rep_ss.str(), WebStatus::WEB_OK);
	}

	bool HttpModule::OnWorldList(std::shared_ptr<HttpRequest> req) {
		AckWorldList ack;
		auto& world_servers = m_master_->GetWorldServers();

		for (auto& iter : world_servers) {
			auto &server = iter.second;
			AckWorldList::World world;
			world.id = server.server_id();
			world.name = server.server_name();
			world.state = server.server_state();
			world.count = server.server_cur_count();
			ack.world.push_back(world);
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
		std::string user = GetUserID(request);
		dout << "request: " << request->body.c_str() << std::endl;
		ajson::load_from_buff(req, request->body.c_str());
		do {
			
			// 判断world id是否存在
			auto& servers = m_master_->GetWorldServers();
			if (servers.find(req.world_id) == servers.end()) {
				dout << "客户端选择world_id错误: " << req.world_id << std::endl;
				ack.code = IResponse::QEUEST_ERROR;
				break;
			}

			// 选择一个workload最小的proxy给客户端
			auto &proxy_servers = m_master_->GetProxyServers();
			auto count = proxy_servers.size();
			if (count < 1) {
				ack.code = IResponse::SERVER_ERROR;
				break;
			}


			// find a server
			int min_proxy_id = proxy_servers.begin()->second.server_id();
			int min_workload = 0;
			for (auto& iter : proxy_servers) {
				auto server = iter.second;
				if (min_workload > server.server_cur_count()) {
					min_proxy_id = iter.first;
				}
			}

			auto server = proxy_servers[min_proxy_id];
			Guid key = m_kernel_->CreateGUID();
			ack.code = IResponse::SUCCESS;
			ack.ip = server.server_ip();
			ack.port = server.server_port();
			ack.world_id = req.world_id;
			ack.guid = user;
			ack.key = key.ToString();
			ack.limit_time = 86400; // 限制一天
		} while (false);
		
		ajson::string_stream rep_ss;
		ajson::save_to(rep_ss, ack);
		dout << "response: " << rep_ss.str() << std::endl;
		return m_http_server_->ResponseMsg(request, rep_ss.str(), WebStatus::WEB_OK);
	}

	std::string HttpModule::GetUserID(std::shared_ptr<HttpRequest> req) {
		auto it = req->headers.find("User");
		if (it != req->headers.end()) {
			return it->second;
		}

		it = req->headers.find("user");
		if (it != req->headers.end()) {
			return it->second;
		}

		return "";
	}

	std::string HttpModule::GetUserJWT(std::shared_ptr<HttpRequest> req) {
		auto it = req->headers.find("Jwt");
		if (it != req->headers.end()) {
			return it->second;
		}

		it = req->headers.find("jwt");
		if (it != req->headers.end()) {
			return it->second;
		}

		return "";
	}

	bool HttpModule::CheckUserJWT(const std::string& user, const std::string& jwt) {
		auto it = tokens_.find(user);
		if (it != tokens_.end()) {
			return (it->second == jwt);
		}
		return false;
	}

	WebStatus HttpModule::OnFilter(std::shared_ptr<HttpRequest> req) {
		std::string user = GetUserID(req);
		std::string jwt = GetUserJWT(req);

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
		
		bool bRet = CheckUserJWT(user, jwt);
		//bool bRet = true;
		if (bRet) {
			return WebStatus::WEB_OK;
		}
		return WebStatus::WEB_AUTH;
	}

	bool HttpModule::OnGetCDN(std::shared_ptr<HttpRequest> req) {

		json repRoot;
		json cdnServerList;

		repRoot["code"] = 0;
		repRoot["msg"] = "";

		std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
		if (xLogicClass) {
			const std::vector<std::string>& strIdList = xLogicClass->GetIDList();
			for (int i = 0; i < strIdList.size(); ++i) {
				const std::string& strId = strIdList[i];
				int type = m_element_->GetPropertyInt32(strId, excel::Server::Type());
				if (type != ServerType::SQUICK_ST_CDN) {
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


