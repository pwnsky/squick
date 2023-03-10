
#include "game_module.h"
#include "plugin.h"
#include <squick/plugin/config/i_class_module.h>
namespace proxy::client {

bool GameModule::Start()
{
	m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
	m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
	m_pProxyServerNet_ServerModule = pPluginManager->FindModule<server::IServerModule>();
	m_pElementModule = pPluginManager->FindModule<IElementModule>();
	m_pLogModule = pPluginManager->FindModule<ILogModule>();
	m_pClassModule = pPluginManager->FindModule<IClassModule>();

    return true;
}

bool GameModule::Destory()
{
    //Final();
    //Clear();
    return true;
}

bool GameModule::Update()
{
	return true;
}

bool GameModule::AfterStart()
{
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_GAME, SquickStruct::GameLobbyRPC::ACK_ENTER, this, &GameModule::OnAckEnterGame);
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_GAME, this, &GameModule::Transport);

	m_pNetClientModule->AddEventCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_GAME, this, &GameModule::OnSocketGSEvent);
	m_pNetClientModule->ExpandBufferSize();

    return true;
}

void GameModule::OnSocketGSEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet* pNet)
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
        Register(pNet);
    }
}

void GameModule::Register(INet* pNet)
{
    SQUICK_SHARE_PTR<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
    if (xLogicClass)
    {
		const std::vector<std::string>& strIdList = xLogicClass->GetIDList();
		for (int i = 0; i < strIdList.size(); ++i)
		{
			const std::string& strId = strIdList[i];

            const int serverType = m_pElementModule->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == SQUICK_SERVER_TYPES::SQUICK_ST_PROXY && pPluginManager->GetAppID() == serverID)
            {
                const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_pElementModule->GetPropertyInt32(strId, excel::Server::MaxOnline());
                //const int nCpus = m_pElementModule->GetPropertyInt32(strId, SquickProtocol::Server::CpuCount());
                const std::string& name = m_pElementModule->GetPropertyString(strId, excel::Server::ID());
                const std::string& ip = m_pElementModule->GetPropertyString(strId, excel::Server::IP());

                SquickStruct::ServerInfoReportList xMsg;
                SquickStruct::ServerInfoReport* pData = xMsg.add_server_list();

                pData->set_server_id(serverID);
                pData->set_server_name(strId);
                pData->set_server_cur_count(0);
                pData->set_server_ip(ip);
                pData->set_server_port(nPort);
                pData->set_server_max_online(maxConnect);
                pData->set_server_state(SquickStruct::ServerState::SERVER_NORMAL);
                pData->set_server_type(serverType);

                SQUICK_SHARE_PTR<ConnectData> pServerData = m_pNetClientModule->GetServerNetInfo(pNet);
                if (pServerData)
                {
                    int nTargetID = pServerData->nGameID;
                    m_pNetClientModule->SendToServerByPB(nTargetID, SquickStruct::ServerRPC::PROXY_TO_GAME_REGISTERED, xMsg);

                    m_pLogModule->LogInfo(Guid(0, pData->server_id()), pData->server_name(), "Register");
                }
            }
        }
    }
}
/**
 * 进入游戏通知玩家
*/
void GameModule::OnAckEnterGame(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
    dout << "进入游戏成功!\n";
    Guid nPlayerID;
    SquickStruct::AckEnter xData;
    if (!INetModule::ReceivePB( msgID, msg, len, xData, nPlayerID)) {
        return;
    }
    
	const Guid& xClient = INetModule::ProtobufToStruct(xData.client());
	const Guid& xPlayer = INetModule::ProtobufToStruct(xData.object());

	m_pProxyServerNet_ServerModule->EnterGameSuccessEvent(xClient, xPlayer);
	m_pProxyServerNet_ServerModule->Transport(sockIndex, msgID, msg, len);
}

void GameModule::LogServerInfo(const std::string& strServerInfo)
{
    m_pLogModule->LogInfo(Guid(), strServerInfo, "");
}

void GameModule::Transport(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
{
	m_pProxyServerNet_ServerModule->Transport(sockIndex, msgID, msg, len);
}


}