

#include "db_module.h"
#include "plugin.h"
#include <squick/struct/struct.h>
#include <squick/plugin/net/i_net_client_module.h>

bool GameServerToDBModule::Start()
{
	m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
	m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
	m_pClassModule = pPluginManager->FindModule<IClassModule>();
	m_pElementModule = pPluginManager->FindModule<IElementModule>();
	m_pLogModule = pPluginManager->FindModule<ILogModule>();
	m_pGameServerNet_ServerModule = pPluginManager->FindModule<IGameServerNet_ServerModule>();

	return true;
}

bool GameServerToDBModule::Destory()
{

	return true;
}


bool GameServerToDBModule::Update()
{
	return true;
}

bool GameServerToDBModule::AfterStart()
{
	//m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, this, &GameServerToDBModule::TransPBToProxy);
	//m_pNetClientModule->AddEventCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, this, &GameServerToDBModule::OnSocketWSEvent);
	
	m_pNetClientModule->ExpandBufferSize();

	return true;
}

void GameServerToDBModule::OnSocketWSEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet* pNet)
{
	if (eEvent & SQUICK_NET_EVENT_EOF)
	{
	}
	else if (eEvent & SQUICK_NET_EVENT_ERROR)
	{
	}
	else if (eEvent & SQUICK_NET_EVENT_TIMEOUT)
	{
	}
	else  if (eEvent & SQUICK_NET_EVENT_CONNECTED)
	{
		m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
	}
}

void GameServerToDBModule::TransPBToProxy(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	m_pNetClientModule->SendBySuitWithOutHead(SQUICK_SERVER_TYPES::SQUICK_ST_DB, sockIndex, msgID, std::string(msg, len));

	return;
}

void GameServerToDBModule::TransmitToDB(const int nHashKey, const int msgID, const google::protobuf::Message& xData)
{
	m_pNetClientModule->SendSuitByPB(SQUICK_SERVER_TYPES::SQUICK_ST_DB, nHashKey, msgID, xData);
}