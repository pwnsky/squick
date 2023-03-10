
#include "hello_http_server_module.h"


bool HelloWorld5::Start()
{ 
	m_pElementModule = pPluginManager->FindModule<IElementModule>();
	m_pScheduleModule = pPluginManager->FindModule<IScheduleModule>();
	m_pLogicClassModule = pPluginManager->FindModule<IClassModule>();
	m_pHttpClientModule = pPluginManager->FindModule<IHttpClientModule>();
	m_pHttpNetModule = pPluginManager->FindModule<IHttpServerModule>();
	m_pWSModule = pPluginManager->FindModule<IWSModule>();
	m_pNetModule = pPluginManager->FindModule<INetModule>();
	
    return true;
}

bool HelloWorld5::AfterStart()
{
	m_pScheduleModule->AddSchedule(Guid(0, 1), "OnHeartBeat1", this, &HelloWorld5::OnHeartBeat, 5.0f, 10);
	m_pScheduleModule->AddSchedule(Guid(0, 1), "OnHeartBeat2", this, &HelloWorld5::OnHeartBeat, 5.0f, 10);

	std::cout << "Hello, world, Start" << std::endl;
	//http://127.0.0.1/json
	m_pHttpNetModule->AddRequestHandler("/json", HttpType::SQUICK_HTTP_REQ_GET, this, &HelloWorld5::OnCommandQuery);
	m_pHttpNetModule->AddRequestHandler("/json", HttpType::SQUICK_HTTP_REQ_POST, this, &HelloWorld5::OnCommandQuery);
	m_pHttpNetModule->AddRequestHandler("/json", HttpType::SQUICK_HTTP_REQ_DELETE, this, &HelloWorld5::OnCommandQuery);
	m_pHttpNetModule->AddRequestHandler("/json", HttpType::SQUICK_HTTP_REQ_PUT, this, &HelloWorld5::OnCommandQuery);

	m_pHttpNetModule->AddNetFilter("/json", this, &HelloWorld5::OnFilter);

	m_pHttpNetModule->StartServer(8080);


    m_pWSModule->Startialization(9999, 8090, 4);

	m_pWSModule->AddReceiveCallBack(this, &HelloWorld5::OnWebSocketTestProcess);

	m_pNetModule->Startialization(9999, 5001);
	m_pNetModule->AddEventCallBack( this, &HelloWorld5::OnTCPEvent);
	m_pNetModule->AddReceiveCallBack(SquickStruct::REQ_LOGIN, this, &HelloWorld5::OnLoginProcess);

    return true;
}

bool HelloWorld5::Update()
{
	if(m_pHttpNetModule)
		m_pHttpNetModule->Update();
 
    return true;
}

bool HelloWorld5::BeforeDestory()
{
    
    std::cout << "Hello, world2, BeforeDestory" << std::endl;

    return true;
}

bool HelloWorld5::Destory()
{
    
    std::cout << "Hello, world2, Destory" << std::endl;

    return true;
}

bool HelloWorld5::OnCommandQuery(SQUICK_SHARE_PTR<HttpRequest> req)
{
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

	return m_pHttpNetModule->ResponseMsg(req, "OnCommandQuery --- test1", WebStatus::WEB_OK);
}

WebStatus HelloWorld5::OnFilter(SQUICK_SHARE_PTR<HttpRequest> req)
{
	std::cout << "OnFilter ... " << std::endl;

	return WebStatus::WEB_OK;
}

int HelloWorld5::OnHeartBeat(const Guid & self, const std::string & heartBeat, const float time, const int count)
{
	std::cout << heartBeat << std::endl;

	m_pHttpClientModule->DoGet("http://127.0.0.1:8080/json", this, &HelloWorld5::OnGetCallBack);
	m_pHttpClientModule->DoGet("http://127.0.0.1:8080/json", [](const Guid id, const int state_code, const std::string & strRespData, const std::string & strMemoData) -> void
	{
		std::cout << "OnGetCallBack" << std::endl;
	});

    std::string strMemo = "Memo here";
	m_pHttpClientModule->DoPost("http://127.0.0.1:8080/json", "OnHeartBeat post data---", this, &HelloWorld5::OnPostCallBack, strMemo);

	m_pHttpClientModule->DoPost("http://127.0.0.1:8080/json", "OnHeartBeat post data---", [](const Guid id, const int state_code, const std::string & strRespData, const std::string & strMemoData) -> void
	{
		std::cout << "OnPostCallBack" << std::endl;
	});

	return 0;
}

void HelloWorld5::OnGetCallBack(const Guid id, const int state_code, const std::string & strRespData)
{
	std::cout << "OnGetCallBack" << std::endl;
}

void HelloWorld5::OnPostCallBack(const Guid id, const int state_code, const std::string& strRespData, const std::string& strMemoData)
{
    std::cout << "OnPostCallBack" << " "<< strMemoData <<std::endl;
}

///////////////////////////////////////web socket ////////////////////////////////////////

void HelloWorld5::OnWebSocketTestProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	std::string s(msg, len);
    std::cout << s << std::endl;
    m_pWSModule->SendMsg(s, sockIndex);
}

void HelloWorld5::OnTCPEvent(const SQUICK_SOCKET fd, const SQUICK_NET_EVENT event, INet * pNet)
{
	std::cout << "fd:" << fd << " event " << event << std::endl;
}

void HelloWorld5::OnLoginProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
{
	Guid nPlayerID;
	SquickStruct::ReqAccountLogin xMsg;
	if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, nPlayerID))
	{
		return;
	}

	std::cout << xMsg.account() << " " << xMsg.password() << std::endl;
}
