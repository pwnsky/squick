

#include "http_server_module.h"
#include <squick/struct/protocol_define.h>
namespace squick::server::www::plugin::server::module {
bool HttpServer::Start()
{
	m_pHttpNetModule = pPluginManager->FindModule<IHttpServerModule>();
	m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
	//m_pMasterServerModule = pPluginManager->FindModule<IMasterNet_ServerModule>();
	m_pLogicClassModule = pPluginManager->FindModule<IClassModule>();
	m_pElementModule = pPluginManager->FindModule<IElementModule>();

	return true;
}
bool HttpServer::Destory()
{
	return true;
	
}

bool HttpServer::AfterStart()
{
    //http://127.0.0.1/json
	m_pHttpNetModule->AddRequestHandler("/status", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpServer::OnCommandQuery);
	m_pHttpNetModule->AddRequestHandler("/status", HttpType::SQUICK_HTTP_REQ_POST, this, &HttpServer::OnCommandQuery);
	m_pHttpNetModule->AddRequestHandler("/status", HttpType::SQUICK_HTTP_REQ_DELETE, this, &HttpServer::OnCommandQuery);
	m_pHttpNetModule->AddRequestHandler("/status", HttpType::SQUICK_HTTP_REQ_PUT, this, &HttpServer::OnCommandQuery);

	m_pHttpNetModule->AddNetFilter("/status", this, &HttpServer::OnFilter);

	SQUICK_SHARE_PTR<IClass> xLogicClass = m_pLogicClassModule->GetElement(SquickProtocol::Server::ThisName());
	if (xLogicClass)
	{
		const std::vector<std::string>& strIdList = xLogicClass->GetIDList();
		for (int i = 0; i < strIdList.size(); ++i)
		{
			const std::string& strId = strIdList[i];

			int nJsonPort = m_pElementModule->GetPropertyInt32(strId, SquickProtocol::Server::WebPort());
			int nWebServerAppID = m_pElementModule->GetPropertyInt32(strId, SquickProtocol::Server::ServerID());

			//webserver only run one instance in each server
			if (pPluginManager->GetAppID() == nWebServerAppID)
			{
				m_pHttpNetModule->StartServer(nJsonPort);

				break;
			}
		}
	}

	return true;
}

bool HttpServer::Update()
{
	//std::cout << "ookkkkk" << std::endl;
    m_pHttpNetModule->Update();
	return true;
}

bool HttpServer::OnCommandQuery(SQUICK_SHARE_PTR<HttpRequest> req)
{
	
	std::cout << "url: " << req->url << std::endl;
	std::cout << "path: " << req->path << std::endl;
	std::cout << "type: " << req->type << std::endl;
	std::cout << "body: " << req->body << std::endl;
	/*
	std::cout << "params: " << std::endl;

	for (auto item : req->params)
	{
		std::cout << item.first << ":" << item.second << std::endl;
	}

    std::cout << "headers: " << std::endl;

	for (auto item : req->headers)
	{
		std::cout << item.first << ":" << item.second << std::endl;
	}*/

	//std::string str = m_pMasterServerModule->GetServersStatus();
	return m_pHttpNetModule->ResponseMsg(req, "aaa", WebStatus::WEB_OK);
}

WebStatus HttpServer::OnFilter(SQUICK_SHARE_PTR<HttpRequest> req)
{
	/*
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
	}*/

	return WebStatus::WEB_OK;
}

}