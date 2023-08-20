
#include "net_module.h"

NetModule::NetModule(IPluginManager *p) {
    is_update_ = true;
    pm_ = p;

    mnBufferSize = 0;
    nLastTime = GetPluginManager()->GetNowTime();
    m_pNet = NULL;
}

NetModule::~NetModule() {
    if (m_pNet) {
        m_pNet->Final();
    }

    delete m_pNet;
    m_pNet = NULL;
}

bool NetModule::Start() {
    m_log_ = pm_->FindModule<ILogModule>();

    return true;
}

bool NetModule::AfterStart() { return true; }

void NetModule::Startialization(const char *ip, const unsigned short nPort) {
    dout << "沃日: 开始连接: " << ip << ": " << nPort << std::endl;
    m_pNet = new Net(this, &NetModule::OnReceiveNetPack, &NetModule::OnSocketNetEvent);
    m_pNet->ExpandBufferSize(mnBufferSize);
    m_pNet->Startialization(ip, nPort);
}

int NetModule::Startialization(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount) {
    m_pNet = new Net(this, &NetModule::OnReceiveNetPack, &NetModule::OnSocketNetEvent);
    m_pNet->ExpandBufferSize(mnBufferSize);
    return m_pNet->Startialization(nMaxClient, nPort, nCpuCount);
}

unsigned int NetModule::ExpandBufferSize(const unsigned int size) {
    if (size > 0) {
        mnBufferSize = size;
        if (m_pNet) {
            m_pNet->ExpandBufferSize(mnBufferSize);
        }
    }

    return mnBufferSize;
}

void NetModule::RemoveReceiveCallBack(const int msg_id) {
    std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::iterator it = mxReceiveCallBack.find(msg_id);
    if (mxReceiveCallBack.end() != it) {
        mxReceiveCallBack.erase(it);
    }
}

bool NetModule::AddReceiveCallBack(const int msg_id, const NET_RECEIVE_FUNCTOR_PTR &cb) {
    if (mxReceiveCallBack.find(msg_id) == mxReceiveCallBack.end()) {
        std::list<NET_RECEIVE_FUNCTOR_PTR> xList;
        xList.push_back(cb);
        mxReceiveCallBack.insert(std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::value_type(msg_id, xList));
        return true;
    }

    std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::iterator it = mxReceiveCallBack.find(msg_id);
    it->second.push_back(cb);

    return true;
}

bool NetModule::AddReceiveCallBack(const NET_RECEIVE_FUNCTOR_PTR &cb) {
    mxCallBackList.push_back(cb);

    return true;
}

bool NetModule::AddEventCallBack(const NET_EVENT_FUNCTOR_PTR &cb) {
    mxEventCallBackList.push_back(cb);

    return true;
}

bool NetModule::Update() {
    if (!m_pNet) {
        return false;
    }

    KeepAlive();

    m_pNet->Update();

    return true;
}

bool NetModule::SendMsgWithOutHead(const int msg_id, const std::string &msg, const socket_t sock) {
    bool bRet = m_pNet->SendMsgWithOutHead(msg_id, msg.c_str(), (uint32_t)msg.length(), sock);
    if (!bRet) {
        std::ostringstream stream;
        stream << " SendMsgWithOutHead failed fd " << sock;
        stream << " msg id " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);
    }

    return bRet;
}

bool NetModule::SendMsgToAllClientWithOutHead(const int msg_id, const std::string &msg) {
    bool bRet = m_pNet->SendMsgToAllClientWithOutHead(msg_id, msg.c_str(), (uint32_t)msg.length());
    if (!bRet) {
        std::ostringstream stream;
        stream << " SendMsgToAllClientWithOutHead failed";
        stream << " msg id " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);
    }

    return bRet;
}

bool NetModule::SendMsgPB(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock, const Guid id) {
    rpc::MsgBase xMsg;
    if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
        std::ostringstream stream;
        stream << " SendMsgPB Message to  " << sock;
        stream << " Failed For Serialize of MsgData, MessageID " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    rpc::Ident *pPlayerID = xMsg.mutable_player_id();
    *pPlayerID = StructToProtobuf(id);

    std::string msg;
    if (!xMsg.SerializeToString(&msg)) {
        std::ostringstream stream;
        stream << " SendMsgPB Message to  " << sock;
        stream << " Failed For Serialize of MsgBase, MessageID " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    return SendMsgWithOutHead(msg_id, msg, sock);
}

bool NetModule::SendMsg(const uint16_t msg_id, const std::string &xData, const socket_t sock) { return SendMsgWithOutHead(msg_id, xData, sock); }

bool NetModule::SendMsg(const uint16_t msg_id, const std::string &xData, const socket_t sock, const Guid id) {
    rpc::MsgBase xMsg;
    xMsg.set_msg_data(xData.data(), xData.length());

    rpc::Ident *pPlayerID = xMsg.mutable_player_id();
    *pPlayerID = StructToProtobuf(id);

    std::string msg;
    if (!xMsg.SerializeToString(&msg)) {
        std::ostringstream stream;
        stream << " SendMsgPB Message to  " << sock;
        stream << " Failed For Serialize of MsgBase, MessageID " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    return SendMsgWithOutHead(msg_id, msg, sock);
}

bool NetModule::SendMsgPB(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock) {
    rpc::MsgBase xMsg;
    if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
        std::ostringstream stream;
        stream << " SendMsgPB Message to  " << sock;
        stream << " Failed For Serialize of MsgData, MessageID " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    rpc::Ident *pPlayerID = xMsg.mutable_player_id();
    *pPlayerID = StructToProtobuf(Guid());

    std::string msg;
    if (!xMsg.SerializeToString(&msg)) {
        std::ostringstream stream;
        stream << " SendMsgPB Message to  " << sock;
        stream << " Failed For Serialize of MsgBase, MessageID " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    SendMsgWithOutHead(msg_id, msg, sock);

    return true;
}

bool NetModule::SendMsgPBToAllClient(const uint16_t msg_id, const google::protobuf::Message &xData) {
    rpc::MsgBase xMsg;
    if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
        std::ostringstream stream;
        stream << " SendMsgPBToAllClient";
        stream << " Failed For Serialize of MsgData, MessageID " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    std::string msg;
    if (!xMsg.SerializeToString(&msg)) {
        std::ostringstream stream;
        stream << " SendMsgPBToAllClient";
        stream << " Failed For Serialize of MsgBase, MessageID " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    return SendMsgToAllClientWithOutHead(msg_id, msg);
}

bool NetModule::SendMsgPB(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock, const std::vector<Guid> *pClientIDList) {
    if (!m_pNet) {
        std::ostringstream stream;
        stream << " m_pNet SendMsgPB faailed fd " << sock;
        stream << " Failed For Serialize of MsgBase, MessageID " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    rpc::MsgBase xMsg;
    if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
        std::ostringstream stream;
        stream << " SendMsgPB faailed fd " << sock;
        stream << " Failed For Serialize of MsgBase, MessageID " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    rpc::Ident *pPlayerID = xMsg.mutable_player_id();
    *pPlayerID = StructToProtobuf(Guid());
    if (pClientIDList) {
        for (int i = 0; i < pClientIDList->size(); ++i) {
            const Guid &ClientID = (*pClientIDList)[i];

            rpc::Ident *pData = xMsg.add_player_client_list();
            if (pData) {
                *pData = StructToProtobuf(ClientID);
            }
        }
    }

    std::string msg;
    if (!xMsg.SerializeToString(&msg)) {
        std::ostringstream stream;
        stream << " SendMsgPB faailed fd " << sock;
        stream << " Failed For Serialize of MsgBase, MessageID " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    return SendMsgWithOutHead(msg_id, msg, sock);
}

bool NetModule::SendMsgPB(const uint16_t msg_id, const std::string &strData, const socket_t sock, const std::vector<Guid> *pClientIDList) {
    if (!m_pNet) {
        std::ostringstream stream;
        stream << " SendMsgPB NULL Of Net faailed fd " << sock;
        stream << " Failed For Serialize of MsgBase, MessageID " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    rpc::MsgBase xMsg;
    xMsg.set_msg_data(strData.data(), strData.length());

    rpc::Ident *pPlayerID = xMsg.mutable_player_id();
    *pPlayerID = StructToProtobuf(Guid());
    if (pClientIDList) {
        for (int i = 0; i < pClientIDList->size(); ++i) {
            const Guid &ClientID = (*pClientIDList)[i];

            rpc::Ident *pData = xMsg.add_player_client_list();
            if (pData) {
                *pData = StructToProtobuf(ClientID);
            }
        }
    }

    std::string msg;
    if (!xMsg.SerializeToString(&msg)) {
        std::ostringstream stream;
        stream << " SendMsgPB failed fd " << sock;
        stream << " Failed For Serialize of MsgBase, MessageID " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    return SendMsgWithOutHead(msg_id, msg, sock);
}

INet *NetModule::GetNet() { return m_pNet; }

void NetModule::OnReceiveNetPack(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    // m_log_->LogInfo(pm_->GetAppName() + std::to_string(pm_->GetAppID()) + " NetModule::OnReceiveNetPack " +
    // std::to_string(msg_id), __FILE__, __LINE__);

    Performance performance;

#if PLATFORM != PLATFORM_WIN
    SQUICK_CRASH_TRY
#endif

    std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::iterator it = mxReceiveCallBack.find(msg_id);
    if (mxReceiveCallBack.end() != it) {
        std::list<NET_RECEIVE_FUNCTOR_PTR> &xFunList = it->second;
        for (std::list<NET_RECEIVE_FUNCTOR_PTR>::iterator itList = xFunList.begin(); itList != xFunList.end(); ++itList) {
            NET_RECEIVE_FUNCTOR_PTR &pFunPtr = *itList;
            NET_RECEIVE_FUNCTOR *pFunc = pFunPtr.get();

            pFunc->operator()(sock, msg_id, msg, len);
        }
    } else {
        for (std::list<NET_RECEIVE_FUNCTOR_PTR>::iterator itList = mxCallBackList.begin(); itList != mxCallBackList.end(); ++itList) {
            NET_RECEIVE_FUNCTOR_PTR &pFunPtr = *itList;
            NET_RECEIVE_FUNCTOR *pFunc = pFunPtr.get();

            pFunc->operator()(sock, msg_id, msg, len);
        }
    }

#if PLATFORM != PLATFORM_WIN
    SQUICK_CRASH_END
#endif
    /*
            if (performance.CheckTimePoint(5))
            {
                    std::ostringstream os;
                    os << "---------------net module performance problem------------------- ";
                    os << performance.TimeScope();
                    os << "---------- MsgID: ";
                    os << msg_id;
                    m_log_->LogWarning(Guid(0, msg_id), os, __FUNCTION__, __LINE__);
            }
     */
}

void NetModule::OnSocketNetEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    for (std::list<NET_EVENT_FUNCTOR_PTR>::iterator it = mxEventCallBackList.begin(); it != mxEventCallBackList.end(); ++it) {
        NET_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        NET_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(sock, eEvent, pNet);
    }
}

void NetModule::KeepAlive() {
    if (!m_pNet) {
        return;
    }

    if (m_pNet->IsServer()) {
        return;
    }

    if (nLastTime + 10 > GetPluginManager()->GetNowTime()) {
        return;
    }

    nLastTime = GetPluginManager()->GetNowTime();

    rpc::ServerHeartBeat xMsg;
    xMsg.set_count(0);

    SendMsgPB(rpc::ServerRPC::SERVER_HEARTBEAT, xMsg, 0);
}