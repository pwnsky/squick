

#include "server_module.h"
#include <squick/plugin/lua/export.h>
#include <third_party/nlohmann/json.hpp>
namespace gateway::server {
bool ServerModule::Start()
{
	this->pPluginManager->SetAppType(SQUICK_SERVER_TYPES::SQUICK_ST_GATEWAY);
	m_pHttpNetModule = pPluginManager->FindModule<IHttpServerModule>();
	m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
	m_pLogicClassModule = pPluginManager->FindModule<IClassModule>();
	m_pElementModule = pPluginManager->FindModule<IElementModule>();
	m_pLuaScriptModule = pPluginManager->FindModule<ILuaScriptModule>();
	return true;
}
bool ServerModule::Destory()
{
	return true;
}

bool ServerModule::AfterStart()
{
	m_pHttpNetModule->AddRequestHandler("/list", HttpType::SQUICK_HTTP_REQ_GET, this, &ServerModule::OnGetServerList);
	
	
	SQUICK_SHARE_PTR<IClass> xLogicClass = m_pLogicClassModule->GetElement(excel::Server::ThisName());
	if (xLogicClass) {
		const std::vector<std::string>& strIdList = xLogicClass->GetIDList();
		for (int i = 0; i < strIdList.size(); ++i)
		{
			const std::string& strId = strIdList[i];

			int nJsonPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::WebPort());
			int nWebServerAppID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());

			//webserver only run one instance in each server
			if (pPluginManager->GetAppID() == nWebServerAppID && nJsonPort > 0)
			{
				std::cout << "port: " << nJsonPort << std::endl;
				m_pHttpNetModule->StartServer(nJsonPort);
				break;
			}
		}
	}
	return true;
}

bool ServerModule::Update()
{
	m_pHttpNetModule->Update();
	return true;
}

bool ServerModule::OnGetServerList(SQUICK_SHARE_PTR<HttpRequest> req)
{
	using json = nlohmann::json;
	json repRoot;
	json loginServerList, proxyServerList, cdnServerList;

	repRoot["code"] = 0;
	repRoot["msg"] = "";
	// 采用一定算法选择最优的三个给客户端。
	proxyServerList = {
		{
		 { "name", "代理服务器节点_01" },
		 { "ip", "127.0.0.1" },
		 { "port", 15001 },
		 { "id", 15001 },
		 { "type", 0 },
		 },
	};

	loginServerList = {
		{
		 { "name", "登录服_01" },
		 { "ip", "127.0.0.1" },
		 { "port", 15001 },
		 { "id", 15001 },
		 { "type", 0 },
		 },
	};

	cdnServerList = {
		{
		 { "name", "cdn服_01" },
		 { "ip", "127.0.0.1" },
		 { "port", 15001 },
		 { "id", 15001 },
		 { "type", 0 },
		 },
	};

	repRoot["servers"] = {
		{"login", loginServerList },
		{"proxy", proxyServerList },
		{"cdn", cdnServerList },
	};
	
	return m_pHttpNetModule->ResponseMsg(req, repRoot.dump(), WebStatus::WEB_OK);
}

}