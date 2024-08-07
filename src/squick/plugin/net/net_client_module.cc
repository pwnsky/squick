
#include "net_client_module.h"
#include "net_module.h"

NetClientModule::NetClientModule(IPluginManager *p) {
    is_update_ = true;
    pm_ = p;
}

bool NetClientModule::Start() {
    m_log_ = pm_->FindModule<ILogModule>();

    for (int i = 0; i < rpc::ST_MAX; ++i) {
        INetClientModule::AddEventCallBack(i, this, &NetClientModule::OnSocketEvent);
        INetClientModule::AddReceiveCallBack(i, this, &NetClientModule::OnAckHandler);
    }
    return true;
}

bool NetClientModule::AfterStart() { return true; }

bool NetClientModule::BeforeDestroy() { return true; }

bool NetClientModule::Destroy() { return true; }

bool NetClientModule::Update() {
    ProcessUpdate();
    ProcessNetConnect();
    return true;
}

void NetClientModule::RemoveReceiveCallBack(const int eType, const uint16_t msg_id) {
    std::shared_ptr<CallBack> xCallBack = mxCallBack.GetElement(eType);
    if (xCallBack) {
        std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::iterator it = xCallBack->mxReceiveCallBack.find(msg_id);
        if (xCallBack->mxReceiveCallBack.end() != it) {
            xCallBack->mxReceiveCallBack.erase(it);
        }
    }
}

void NetClientModule::AddNode(const ConnectData &xInfo) { mxTempNetList.push_back(xInfo); }

int NetClientModule::AddReceiveCallBack(const int eType, const uint16_t msg_id, NET_RECEIVE_FUNCTOR_PTR functorPtr) {
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

int NetClientModule::AddReceiveCallBack(const int eType, NET_RECEIVE_FUNCTOR_PTR functorPtr) {
    std::shared_ptr<CallBack> xCallBack = mxCallBack.GetElement(eType);
    if (!xCallBack) {
        xCallBack = std::shared_ptr<CallBack>(new CallBack);
        mxCallBack.AddElement(eType, xCallBack);
    }

    xCallBack->mxCallBackList.push_back(functorPtr);

    return 0;
}

int NetClientModule::AddEventCallBack(const int eType, NET_EVENT_FUNCTOR_PTR functorPtr) {
    std::shared_ptr<CallBack> xCallBack = mxCallBack.GetElement(eType);
    if (!xCallBack) {
        xCallBack = std::shared_ptr<CallBack>(new CallBack);
        mxCallBack.AddElement(eType, xCallBack);
    }

    xCallBack->mxEventCallBack.push_back(functorPtr);
    return 0;
}
bool NetClientModule::IsConnected(const int node_id) {
    std::shared_ptr<ConnectData> pServer = mxServerMap.GetElement(node_id);
    ;
    if (pServer) {
        if (pServer->state == ConnectDataState::NORMAL) {
            return true;
        }
    }
    return false;
}

bool NetClientModule::SendByID(const int serverID, const uint16_t msg_id, const std::string &strData, const uint64_t uid, reqid_t req_id) {
    std::shared_ptr<ConnectData> pServer = mxServerMap.GetElement(serverID);
    if (pServer) {
        std::shared_ptr<INetModule> pNetModule = pServer->net_module;
        if (pNetModule.get()) {
            // If sock is 0, that it just working on net client
            if (!pNetModule->SendToNode(msg_id, strData, 0, uid, req_id)) {
                LOG_ERROR("SendToNode failed, node_id<%v> msg_id<%v> ", pServer->id, msg_id);
                return false;
            }
        }
    } else {
        LOG_ERROR("Can't find the node_id<%v>, msg_id<%v>", serverID, msg_id);
        return false;
    }
    return true;
}

void NetClientModule::SendToAllNode(const uint16_t msg_id, const std::string &strData, const uint64_t uid) {
    std::shared_ptr<ConnectData> pServer = mxServerMap.First();
    while (pServer) {
        std::shared_ptr<INetModule> pNetModule = pServer->net_module;
        if (pNetModule) {
            if (!pNetModule->SendToNode(msg_id, strData, 0, uid)) {
                LOG_ERROR("SendToNode failed, node_id<%v> msg_id<%v> ", pServer->id, msg_id);
            }
        }

        pServer = mxServerMap.Next();
    }
}

void NetClientModule::SendToAllNodeByType(const int eType, const uint16_t msg_id, const std::string &strData, const uint64_t uid) {
    std::shared_ptr<ConnectData> pServer = mxServerMap.First();
    while (pServer) {
        std::shared_ptr<INetModule> pNetModule = pServer->net_module;
        if (pNetModule && eType == pServer->type) {
            if (!pNetModule->SendToNode(msg_id, strData, 0, uid)) {
                LOG_ERROR("SendToNode failed, node_id<%v> msg_id<%v> ", pServer->id, msg_id);
            }
        }

        pServer = mxServerMap.Next();
    }
}

bool NetClientModule::SendPBByID(const int serverID, const uint16_t msg_id, const google::protobuf::Message &xData, const uint64_t uid, reqid_t req_id) {
    std::shared_ptr<ConnectData> pServer = mxServerMap.GetElement(serverID);
    if (pServer) {
        std::shared_ptr<INetModule> pNetModule = pServer->net_module;
        if (pNetModule) {
            if (!pNetModule->SendPBToNode(msg_id, xData, 0, uid, req_id)) {
                LOG_ERROR("SendPBToNode failed, node_id<%v> msg_id<%v> ", pServer->id, msg_id);
                return false;
            }
        }
    } else {
        LOG_ERROR("Can't find the node_id<%v>, msg_id<%v>", serverID, msg_id);
        return false;
    }
    return true;
}

void NetClientModule::SendPBToAllNode(const uint16_t msg_id, const google::protobuf::Message &xData, const uint64_t uid) {
    std::shared_ptr<ConnectData> pServer = mxServerMap.First();
    while (pServer) {
        std::shared_ptr<INetModule> pNetModule = pServer->net_module;
        if (pNetModule) {
            if (!pNetModule->SendPBToNode(msg_id, xData, 0, uid)) {
                LOG_ERROR("SendPBToNode failed, node_id<%v> msg_id<%v> ", pServer->id, msg_id);
            }
        }

        pServer = mxServerMap.Next();
    }
}

void NetClientModule::SendPBToAllNodeByType(const int eType, const uint16_t msg_id, const google::protobuf::Message &xData, const uint64_t uid) {
    std::shared_ptr<ConnectData> pServer = mxServerMap.First();
    while (pServer) {
        std::shared_ptr<INetModule> pNetModule = pServer->net_module;
        if (pNetModule && eType == pServer->type && pServer->state == ConnectDataState::NORMAL) {
            if (!pNetModule->SendPBToNode(msg_id, xData, 0, uid)) {
                LOG_ERROR("SendPBToNode failed, node_id<%v> msg_id<%v> ", pServer->id, msg_id);
            }
        }

        pServer = mxServerMap.Next();
    }
}

Awaitable<NetClientResponseData> NetClientModule::Request(const int node_id, const uint16_t msg_id, const std::string &data, int ack_msg_id,
                                                          const uint64_t uid) {
    Awaitable<NetClientResponseData> awaitbale;

    auto req_id = GenerateRequestID();
    awaitbale.data_.error = true;
    awaitbale.data_.req_id = req_id;
    awaitbale.handler_ = std::bind(&NetClientModule::CoroutineBinder, this, placeholders::_1);
    // send
    SendByID(node_id, msg_id, data, uid, req_id);
    return awaitbale;
}

Awaitable<NetClientResponseData> NetClientModule::RequestPB(const int node_id, const uint16_t msg_id, const google::protobuf::Message &pb, int ack_msg_id,
                                                            const uint64_t uid) {
    Awaitable<NetClientResponseData> awaitbale;

    auto req_id = GenerateRequestID();
    awaitbale.data_.error = true;
    awaitbale.data_.req_id = req_id;
    awaitbale.handler_ = std::bind(&NetClientModule::CoroutineBinder, this, placeholders::_1);
    // send
    SendPBByID(node_id, msg_id, pb, uid, req_id);
    return awaitbale;
}

void NetClientModule::CoroutineBinder(Awaitable<NetClientResponseData> *awaitble) {
    if (awaitble == nullptr) {
        LOG_ERROR("CoroutineBinder %v", "awaitble is nullptr");
        return;
    }
    reqid_t req_id = awaitble->data_.req_id;
    auto iter = co_awaitbles_.find(req_id);
    if (iter == co_awaitbles_.end()) {
        co_awaitbles_[req_id] = awaitble;
    } else {
        LOG_ERROR("CoroutineBinder: same req id in a map, req id<%v> address<%v> ", req_id, awaitble->coro_handle_.address());
    }
}

reqid_t NetClientModule::GenerateRequestID() {
    last_req_id_++;
    if (last_req_id_ > 0xffffffffffff) {
        last_req_id_ = 1;
    }
    return last_req_id_;
}

void NetClientModule::OnAckHandler(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    rpc::MsgBase msg_base;
    if (!msg_base.ParseFromArray(msg, len)) {
        return;
    }
    reqid_t req_id = msg_base.req_id();
    if (req_id == 0)
        return;

    auto iter = co_awaitbles_.find(req_id);
    if (iter == co_awaitbles_.end()) {
        LOG_ERROR("CoroutineBinder: Not find req id in a map, req id<%v>", req_id);
        return;
    }

    auto awaitbale = iter->second;
    awaitbale->data_.error = false;
    awaitbale->data_.ack_msg_id = msg_id;
    awaitbale->data_.data = msg;
    awaitbale->data_.length = len;

    // resume coroutine
    if (!awaitbale->coro_handle_.done()) {
        awaitbale->coro_handle_.resume();
    }

    // remove request infos
    co_awaitbles_.erase(iter);
    return;
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

int NetClientModule::GetRandomNodeID(int node_type) {
    std::shared_ptr<ConsistentHashMapEx<int, ConnectData>> xConnectDataMap = mxServerTypeMap.GetElement(node_type);
    if (!xConnectDataMap) {
        return 0;
    }
    auto node = xConnectDataMap->GetElementBySuitRandom();
    return node->id;
}

void NetClientModule::StartCallBacks(std::shared_ptr<ConnectData> pServerData) {
    std::ostringstream stream;
    stream << "Adding Node, NodeType: " << pServerData->type << " Node ID : " << pServerData->id << " State : " << pServerData->state
           << " IP: " << pServerData->ip << " Port: " << pServerData->port;
    LOG_INFO("%v", stream.str());

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

            pServerData->net_module->Connect(pServerData->ip.c_str(), pServerData->port, pServerData->buffer_size);

            StartCallBacks(pServerData);
        } break;
        default:
            break;
        }

        pServerData = mxServerMap.Next();
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

void NetClientModule::ProcessNetConnect() {

    if (mxTempNetList.size() > 0) {
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
                sd->buffer_size = cd.buffer_size;

                sd->net_module = std::shared_ptr<INetModule>(new NetModule(pm_));

                sd->net_module->Awake();
                sd->net_module->Start();
                sd->net_module->AfterStart();
                sd->net_module->ReadyUpdate();

                sd->net_module->Connect(sd->ip.c_str(), sd->port, sd->buffer_size);

                StartCallBacks(sd);

                mxServerMap.AddElement(cd.id, sd);
            } else {
                // cannot connect one id twice
            }
        }

        mxTempNetList.clear();
    }
}

int NetClientModule::GetConnections() { return mxServerMap.Count(); }