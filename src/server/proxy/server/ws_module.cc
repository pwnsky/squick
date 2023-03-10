
#include "ws_module.h"
#include "plugin.h"
#include <squick/plugin/kernel/i_kernel_module.h>
namespace proxy::server {
    bool ProxyServerNet_WSModule::Start()
    {
        m_pWSModule = pPluginManager->FindModule<IWSModule>();
        m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
        m_pClassModule = pPluginManager->FindModule<IClassModule>();
        m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
        m_pLogModule = pPluginManager->FindModule<ILogModule>();
        m_pElementModule = pPluginManager->FindModule<IElementModule>();
        m_pProxyToWorldModule = pPluginManager->FindModule<client::IWorldModule>();
        m_pSecurityModule = pPluginManager->FindModule<ISecurityModule>();

        return true;
    }

    bool ProxyServerNet_WSModule::AfterStart()
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
                    const int port = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
                    const int maxConnect = m_pElementModule->GetPropertyInt32(strId, excel::Server::MaxOnline());
                    const int cpuCount = m_pElementModule->GetPropertyInt32(strId, excel::Server::CpuCount());
                    //const std::string& name = m_pElementModule->GetPropertyString(strId, SquickProtocol::Server::ID());
                    //const std::string& ip = m_pElementModule->GetPropertyString(strId, SquickProtocol::Server::IP());
                    int wsPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::WSPort());

                    //web server only run one instance in each server
                    if (wsPort > 0)
                    {
                        int nRet = m_pWSModule->Startialization(maxConnect, wsPort, cpuCount);
                        if (nRet < 0)
                        {
                            std::ostringstream strLog;
                            strLog << "Cannot init websocket server net, Port = " << wsPort;
                            m_pLogModule->LogError(NULL_OBJECT, strLog, __FUNCTION__, __LINE__);
                            NFASSERT(nRet, "Cannot init websocket server net", __FILE__, __FUNCTION__);
                            exit(0);
                        }

                        m_pWSModule->AddEventCallBack(this, &ProxyServerNet_WSModule::OnSocketClientEvent);
                        m_pWSModule->AddReceiveCallBack(this, &ProxyServerNet_WSModule::OnWebSocketTestProcess);
                        break;
                    }

                }
            }
        }

        return true;
    }

    bool ProxyServerNet_WSModule::Destory()
    {
        return true;
    }

    bool ProxyServerNet_WSModule::Update()
    {
        return true;
    }

    void ProxyServerNet_WSModule::OnWebSocketTestProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
    {
        std::cout << "Simple WebSocket Test Chat\n";
        m_pWSModule->SendMsgToAllClient(std::string(msg, len));
    }

    void ProxyServerNet_WSModule::OnSocketClientEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet* pNet)
    {
        if (eEvent & SQUICK_NET_EVENT_EOF)
        {
            m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_EOF Connection closed", __FUNCTION__, __LINE__);
            OnClientDisconnect(sockIndex);
        }
        else if (eEvent & SQUICK_NET_EVENT_ERROR)
        {
            m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_ERROR Got an error on the connection", __FUNCTION__, __LINE__);
            OnClientDisconnect(sockIndex);
        }
        else if (eEvent & SQUICK_NET_EVENT_TIMEOUT)
        {
            m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_TIMEOUT read timeout", __FUNCTION__, __LINE__);
            OnClientDisconnect(sockIndex);
        }
        else  if (eEvent & SQUICK_NET_EVENT_CONNECTED)
        {
            m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
            OnClientConnected(sockIndex);
        }
    }

    void ProxyServerNet_WSModule::OnClientDisconnect(const SQUICK_SOCKET nAddress)
    {
        NetObject* pNetObject = m_pWSModule->GetNet()->GetNetObject(nAddress);
        if (pNetObject)
        {
            int nGameID = pNetObject->GetGameID();
            if (nGameID > 0)
            {
                //when a net-object bind a account then tell that game-server
                if (!pNetObject->GetUserID().IsNull())
                {
                    /*
                    SquickStruct::ReqLeaveGameServer xData;
                    xData.set_arg(nGameID);

                    SquickStruct::MsgBase xMsg;

                    //real user id
                    *xMsg.mutable_player_id() = INetModule::StructToProtobuf(pNetObject->GetUserID());

                    if (!xData.SerializeToString(xMsg.mutable_msg_data()))
                    {
                        return;
                    }

                    std::string msg;
                    if (!xMsg.SerializeToString(&msg))
                    {
                        return;
                    }

                    m_pNetClientModule->SendByServerIDWithOutHead(nGameID, SquickStruct::EGameMsgID::REQ_LEAVE_GAME, msg);
                     */

                }
            }

            mxClientIdent.RemoveElement(pNetObject->GetClientID());
        }
    }

    void ProxyServerNet_WSModule::OnClientConnected(const SQUICK_SOCKET nAddress)
    {
        //bind client'id with socket id
        NetObject* pNetObject = m_pWSModule->GetNet()->GetNetObject(nAddress);
        if (pNetObject && pNetObject->GetClientID().IsNull())
        {
            Guid xClientIdent = m_pKernelModule->CreateGUID();
            pNetObject->SetClientID(xClientIdent);
            mxClientIdent.AddElement(xClientIdent, SQUICK_SHARE_PTR<SQUICK_SOCKET>(new SQUICK_SOCKET(nAddress)));


            // 1. create a tcp client to connect to the TCP service provided by proxy server.
            // 2. transfer the ws data come from websocket to the TCP service provided by proxy server.
            // 3.transfer the tcp data come from proxy server to the websocket service to send to clients.
        }
    }

}