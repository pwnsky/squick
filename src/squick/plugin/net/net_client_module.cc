
#include "net_client_module.h"
#include "net_module.h"

NetClientModule::NetClientModule(IPluginManager *p) {
    is_update_ = true;
    mnBufferSize = 0;
    pm_ = p;
}

bool NetClientModule::Start() {
    m_log_ = pm_->FindModule<ILogModule>();

    for (int i = 0; i < ServerType::ST_MAX; ++i) {
        INetClientModule::AddEventCallBack((ServerType)i, this, &NetClientModule::OnSocketEvent);
    }
    return true;
}

bool NetClientModule::AfterStart() { return true; }

bool NetClientModule::BeforeDestory() { return true; }

bool NetClientModule::Destory() { return true; }

bool NetClientModule::Update() {
    ProcessUpdate();
    ProcessAddNetConnect();
    return true;
}

void NetClientModule::RemoveReceiveCallBack(const ServerType eType, const uint16_t msg_id) {
    std::shared_ptr<CallBack> xCallBack = mxCallBack.GetElement(eType);
    if (xCallBack) {
        std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::iterator it = xCallBack->mxReceiveCallBack.find(msg_id);
        if (xCallBack->mxReceiveCallBack.end() != it) {
            xCallBack->mxReceiveCallBack.erase(it);
        }
    }
}

void NetClientModule::AddNode(const ConnectData &xInfo) { mxTempNetList.push_back(xInfo); }

unsigned int NetClientModule::ExpandBufferSize(const unsigned int size) {
    if (size > 0) {
        mnBufferSize = size;
    }
    return mnBufferSize;
}

int NetClientModule::AddReceiveCallBack(const ServerType eType, const uint16_t msg_id, NET_RECEIVE_FUNCTOR_PTR functorPtr) {
    std::shared_ptr<CallBack> xCallBack = mxCallBack.GetElement(eType);
    if (!xCallBack) {
        xCallBack = std::shared_ptr<CallBack>(new CallBack);
        mxCallBack.AddElement(eType, xCallBack);
    }

    std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::iterator itor = xCallBack->mxReceiveCallBack.find(msg_id);
    if (itor == xCallBack->mxReceiveCallBack.end()) {
        std::list<NET_RECEIVE_FUNCTOR_PTR> xList;
        xList.push_back(functorPtr);

        xCallBack->mxReceiveCallBack.insert(std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::value_type(msg_id, xList));
    } else {
        xCallBack->mxReceiveCallBack.at(msg_id).push_back(functorPtr);
    }

    return 0;
}

int NetClientModule::AddReceiveCallBack(const ServerType eType, NET_RECEIVE_FUNCTOR_PTR functorPtr) {
    std::shared_ptr<CallBack> xCallBack = mxCallBack.GetElement(eType);
    if (!xCallBack) {
        xCallBack = std::shared_ptr<CallBack>(new CallBack);
        mxCallBack.AddElement(eType, xCallBack);
    }

    xCallBack->mxCallBackList.push_back(functorPtr);

    return 0;
}

int NetClientModule::AddEventCallBack(const ServerType eType, NET_EVENT_FUNCTOR_PTR functorPtr) {
    std::shared_ptr<CallBack> xCallBack = mxCallBack.GetElement(eType);
    if (!xCallBack) {
        xCallBack = std::shared_ptr<CallBack>(new CallBack);
        mxCallBack.AddElement(eType, xCallBack);
    }

    xCallBack->mxEventCallBack.push_back(functorPtr);
    return 0;
}
bool NetClientModule::IsConnected(const int node_id) {
    std::shared_ptr<ConnectData> pServer = mxServerMap.GetElement(node_id);;
    if (pServer) {
        if (pServer->state == ConnectDataState::NORMAL) {
            return true;
        }
    }
    return false;
}

bool NetClientModule::SendByID(const int serverID, const uint16_t msg_id, const std::string &strData, const string guid) {
    std::shared_ptr<ConnectData> pServer = mxServerMap.GetElement(serverID);
    if (pServer) {
        std::shared_ptr<INetModule> pNetModule = pServer->net_module;
        if (pNetModule.get()) {
            if (!pNetModule->SendMsg(msg_id, strData, 0, guid)) {
                std::ostringstream stream;
                stream << " SendMsgWithOutHead failed " << serverID;
                stream << " msg id " << msg_id;
                m_log_->LogError(stream, __FUNCTION__, __LINE__);
                return false;
            }
        }
    } else {
        std::ostringstream stream;
        stream << " can't find the server " << serverID;
        stream << " msg id " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);
        return false;
    }
    return true;
}

void NetClientModule::SendToAllNode(const uint16_t msg_id, const std::string &strData, const string guid) {
    std::shared_ptr<ConnectData> pServer = mxServerMap.First();
    while (pServer) {
        std::shared_ptr<INetModule> pNetModule = pServer->net_module;
        if (pNetModule) {
            if (!pNetModule->SendMsg(msg_id, strData, 0, guid)) {
                std::ostringstream stream;
                stream << " SendMsgWithOutHead failed " << pServer->id;
                stream << " msg id " << msg_id;
                m_log_->LogError(stream, __FUNCTION__, __LINE__);
            }
        }

        pServer = mxServerMap.Next();
    }
}

void NetClientModule::SendToAllNodeByType(const ServerType eType, const uint16_t msg_id, const std::string &strData, const string guid) {
    std::shared_ptr<ConnectData> pServer = mxServerMap.First();
    while (pServer) {
        std::shared_ptr<INetModule> pNetModule = pServer->net_module;
        if (pNetModule && eType == pServer->type) {
            if (!pNetModule->SendMsg(msg_id, strData, 0, guid)) {
                std::ostringstream stream;
                stream << " SendMsgWithOutHead failed " << pServer->id;
                stream << " msg id " << msg_id;
                m_log_->LogError(stream, __FUNCTION__, __LINE__);
            }
        }

        pServer = mxServerMap.Next();
    }
}

bool NetClientModule::SendPBByID(const int serverID, const uint16_t msg_id, const google::protobuf::Message &xData, const string guid) {
    std::shared_ptr<ConnectData> pServer = mxServerMap.GetElement(serverID);
    if (pServer) {
        std::shared_ptr<INetModule> pNetModule = pServer->net_module;
        if (pNetModule) {
            if (!pNetModule->SendMsgPB(msg_id, xData, 0, guid)) {
                std::ostringstream stream;
                stream << " SendMsgPB failed " << pServer->id;
                stream << " msg id " << msg_id;
                m_log_->LogError(stream, __FUNCTION__, __LINE__);
                return false;
            }
        }
    } else {
        std::ostringstream stream;
        stream << " can't find the server " << serverID;
        stream << " msg id " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);
        return false;
    }
    return true;
}

void NetClientModule::SendPBToAllNode(const uint16_t msg_id, const google::protobuf::Message &xData, const string guid) {
    std::shared_ptr<ConnectData> pServer = mxServerMap.First();
    while (pServer) {
        std::shared_ptr<INetModule> pNetModule = pServer->net_module;
        if (pNetModule) {
            if (!pNetModule->SendMsgPB(msg_id, xData, 0, guid)) {
                std::ostringstream stream;
                stream << " SendMsgPB failed " << pServer->id;
                stream << " msg id " << msg_id;
                m_log_->LogError(stream, __FUNCTION__, __LINE__);
            }
        }

        pServer = mxServerMap.Next();
    }
}

void NetClientModule::SendPBToAllNodeByType(const ServerType eType, const uint16_t msg_id, const google::protobuf::Message &xData, const string guid) {
    std::shared_ptr<ConnectData> pServer = mxServerMap.First();
    while (pServer) {
        std::shared_ptr<INetModule> pNetModule = pServer->net_module;
        if (pNetModule && eType == pServer->type && pServer->state == ConnectDataState::NORMAL) {
            if (!pNetModule->SendMsgPB(msg_id, xData, 0, guid)) {
                std::ostringstream stream;
                stream << " SendMsgPB failed " << pServer->id;
                stream << " msg id " << msg_id;
                m_log_->LogError(stream, __FUNCTION__, __LINE__);
            }
        }

        pServer = mxServerMap.Next();
    }
}

Awaitable<NetClientResponseData>  NetClientModule::RequestByID(const int serverID, const uint16_t msg_id, const std::string& strData, int ack_msg_id) {
    Awaitable<NetClientResponseData> a;
    return a;
}

std::shared_ptr<ConnectData> NetClientModule::GetServerNetInfo(const ServerType eType) {
    std::shared_ptr<ConsistentHashMapEx<int, ConnectData>> xConnectDataMap = mxServerTypeMap.GetElement(eType);
    if (xConnectDataMap) {
        return xConnectDataMap->GetElementBySuitRandom();
    }

    return nullptr;
}

std::shared_ptr<ConnectData> NetClientModule::GetServerNetInfo(const int serverID) { return mxServerMap.GetElement(serverID); }

MapEx<int, ConnectData> &NetClientModule::GetServerList() { return mxServerMap; }

std::shared_ptr<ConnectData> NetClientModule::GetServerNetInfo(const INet *pNet) {
    int serverID = 0;
    for (std::shared_ptr<ConnectData> pData = mxServerMap.First(serverID); pData != NULL; pData = mxServerMap.Next(serverID)) {
        if (pData->net_module && pNet == pData->net_module->GetNet()) {
            return pData;
        }
    }

    return std::shared_ptr<ConnectData>(NULL);
}

void NetClientModule::StartCallBacks(std::shared_ptr<ConnectData> pServerData) {
    std::ostringstream stream;
    stream << "Adding Node, NodeType: " << pServerData->type << " Node ID : " << pServerData->id << " State : " << pServerData->state
           << " IP: " << pServerData->ip << " Port: " << pServerData->port << "\n";

    m_log_->LogInfo(stream.str());

    std::shared_ptr<CallBack> xCallBack = mxCallBack.GetElement(pServerData->type);
    if (!xCallBack) {
        xCallBack = std::shared_ptr<CallBack>(new CallBack);
        mxCallBack.AddElement(pServerData->type, xCallBack);
    }

    // add msg callback
    std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::iterator itReciveCB = xCallBack->mxReceiveCallBack.begin();
    for (; xCallBack->mxReceiveCallBack.end() != itReciveCB; ++itReciveCB) {
        std::list<NET_RECEIVE_FUNCTOR_PTR> &xList = itReciveCB->second;
        for (std::list<NET_RECEIVE_FUNCTOR_PTR>::iterator itList = xList.begin(); itList != xList.end(); ++itList) {
            pServerData->net_module->AddReceiveCallBack(itReciveCB->first, *itList);
        }
    }

    // add event callback
    std::list<NET_EVENT_FUNCTOR_PTR>::iterator itEventCB = xCallBack->mxEventCallBack.begin();
    for (; xCallBack->mxEventCallBack.end() != itEventCB; ++itEventCB) {
        pServerData->net_module->AddEventCallBack(*itEventCB);
    }

    std::list<NET_RECEIVE_FUNCTOR_PTR>::iterator itCB = xCallBack->mxCallBackList.begin();
    for (; xCallBack->mxCallBackList.end() != itCB; ++itCB) {
        pServerData->net_module->AddReceiveCallBack(*itCB);
    }
}

void NetClientModule::ProcessUpdate() {
    std::shared_ptr<ConnectData> pServerData = mxServerMap.First();
    while (pServerData) {
        switch (pServerData->state) {
        case ConnectDataState::DISCONNECT: {
            if (NULL != pServerData->net_module) {
                pServerData->net_module = nullptr;
                pServerData->state = ConnectDataState::RECONNECT;
            }
        } break;
        case ConnectDataState::CONNECTING: {
            if (pServerData->net_module) {
                pServerData->net_module->Update();
            }
        } break;
        case ConnectDataState::NORMAL: {
            if (pServerData->net_module) {
                pServerData->net_module->Update();

                KeepState(pServerData);
            }
        } break;
        case ConnectDataState::RECONNECT: {
            if ((pServerData->last_time + 10) >= GetPluginManager()->GetNowTime()) {
                break;
            }

            if (nullptr != pServerData->net_module) {
                pServerData->net_module = nullptr;
            }

            pServerData->state = ConnectDataState::CONNECTING;
            pServerData->net_module = std::shared_ptr<INetModule>(new NetModule(pm_));

            pServerData->net_module->Awake();
            pServerData->net_module->Start();
            pServerData->net_module->AfterStart();
            pServerData->net_module->ReadyUpdate();

            pServerData->net_module->Startialization(pServerData->ip.c_str(), pServerData->port);

            StartCallBacks(pServerData);
        } break;
        default:
            break;
        }

        pServerData = mxServerMap.Next();
    }
}

void NetClientModule::LogServerInfo() {
    bool error = false;
    std::ostringstream stream;
    // stream << "This is a client, begin to print Server Info-------------------" << std::endl;

    ConnectData *pServerData = mxServerMap.FirstNude();
    while (nullptr != pServerData) {
        stream << "\nNetClientModule::LogServerInfo:\nServer Info: Type: " << typeid(pServerData->type).name() << " Server ID: " << pServerData->id
               << " State: " << pServerData->state << " IP: " << pServerData->ip << " Port: " << pServerData->port << std::endl;

        if (pServerData->state != ConnectDataState::NORMAL) {
            error = true;
        }

        pServerData = mxServerMap.NextNude();
    }

    // stream << "\nThis is a client, end to print Server Info--------------------- " << std::endl;

    if (error) {
        // stream << " in " << SQUICK_DEBUG_INFO << std::endl;
        m_log_->LogError(stream.str());
    } else {
        m_log_->LogInfo(stream.str());
    }
}

void NetClientModule::KeepState(std::shared_ptr<ConnectData> pServerData) {
    if (pServerData->last_time + 10 > GetPluginManager()->GetNowTime()) {
        return;
    }

    pServerData->last_time = GetPluginManager()->GetNowTime();

    // send message
}

void NetClientModule::OnSocketEvent(const socket_t fd, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    if (eEvent & SQUICK_NET_EVENT::SQUICK_NET_EVENT_CONNECTED) {
        OnConnected(fd, pNet);
    } else {
        OnDisConnected(fd, pNet);
    }
}

int NetClientModule::OnConnected(const socket_t fd, INet *pNet) {
    std::shared_ptr<ConnectData> pServerInfo = GetServerNetInfo(pNet);
    if (pServerInfo) {
        /////////////////////////////////////////////////////////////////////////////////////
        // AddServerWeightData(pServerInfo);
        pServerInfo->state = ConnectDataState::NORMAL;

        // for type--suit
        std::shared_ptr<ConsistentHashMapEx<int, ConnectData>> xConnectDataMap = mxServerTypeMap.GetElement(pServerInfo->type);
        if (!xConnectDataMap) {
            xConnectDataMap = std::shared_ptr<ConsistentHashMapEx<int, ConnectData>>(new ConsistentHashMapEx<int, ConnectData>());
            mxServerTypeMap.AddElement(pServerInfo->type, xConnectDataMap);
        }

        xConnectDataMap->AddElement(pServerInfo->id, pServerInfo);
    }

    return 0;
}

int NetClientModule::OnDisConnected(const socket_t fd, INet *pNet) {
    std::shared_ptr<ConnectData> pServerInfo = GetServerNetInfo(pNet);
    if (nullptr != pServerInfo) {
        /////////////////////////////////////////////////////////////////////////////////////
        // RemoveServerWeightData(pServerInfo);
        pServerInfo->state = ConnectDataState::DISCONNECT;
        pServerInfo->last_time = GetPluginManager()->GetNowTime();

        // for type--suit
        std::shared_ptr<ConsistentHashMapEx<int, ConnectData>> xConnectDataMap = mxServerTypeMap.GetElement(pServerInfo->type);
        if (xConnectDataMap) {
            xConnectDataMap->RemoveElement(pServerInfo->id);
        }
    }

    return 0;
}

void NetClientModule::ProcessAddNetConnect() {
    std::list<ConnectData>::iterator it = mxTempNetList.begin();
    for (; it != mxTempNetList.end(); ++it) {
        const ConnectData &cd = *it;
        std::shared_ptr<ConnectData> sd = mxServerMap.GetElement(cd.id);
        if (nullptr == sd) {
            sd = std::shared_ptr<ConnectData>(new ConnectData());

            sd->id = cd.id;
            sd->type = cd.type;
            sd->ip = cd.ip;
            sd->name = cd.name;
            sd->state = ConnectDataState::CONNECTING;
            sd->port = cd.port;
            sd->last_time = GetPluginManager()->GetNowTime();

            sd->net_module = std::shared_ptr<INetModule>(new NetModule(pm_));

            sd->net_module->Awake();
            sd->net_module->Start();
            sd->net_module->AfterStart();
            sd->net_module->ReadyUpdate();

            sd->net_module->Startialization(sd->ip.c_str(), sd->port);
            sd->net_module->ExpandBufferSize((unsigned int)mnBufferSize);

            StartCallBacks(sd);

            mxServerMap.AddElement(cd.id, sd);
        } else {
            
        }
    }

    mxTempNetList.clear();
}