
#include "net_module.h"

NetModule::NetModule(IPluginManager *p) {
    m_bIsUpdate = true;
    pPluginManager = p;

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
    m_pLogModule = pPluginManager->FindModule<ILogModule>();

    return true;
}

bool NetModule::AfterStart() { return true; }

void NetModule::Startialization(const char *ip, const unsigned short nPort) {
    m_pNet = SQUICK_NEW Net(this, &NetModule::OnReceiveNetPack, &NetModule::OnSocketNetEvent);
    m_pNet->ExpandBufferSize(mnBufferSize);
    m_pNet->Startialization(ip, nPort);
}

int NetModule::Startialization(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount) {
    m_pNet = SQUICK_NEW Net(this, &NetModule::OnReceiveNetPack, &NetModule::OnSocketNetEvent);
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

void NetModule::RemoveReceiveCallBack(const int msgID) {
    std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::iterator it = mxReceiveCallBack.find(msgID);
    if (mxReceiveCallBack.end() != it) {
        mxReceiveCallBack.erase(it);
    }
}

bool NetModule::AddReceiveCallBack(const int msgID, const NET_RECEIVE_FUNCTOR_PTR &cb) {
    if (mxReceiveCallBack.find(msgID) == mxReceiveCallBack.end()) {
        std::list<NET_RECEIVE_FUNCTOR_PTR> xList;
        xList.push_back(cb);
        mxReceiveCallBack.insert(std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::value_type(msgID, xList));
        return true;
    }

    std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::iterator it = mxReceiveCallBack.find(msgID);
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

bool NetModule::SendMsgWithOutHead(const int msgID, const std::string &msg, const SQUICK_SOCKET sockIndex) {
    bool bRet = m_pNet->SendMsgWithOutHead(msgID, msg.c_str(), (uint32_t)msg.length(), sockIndex);
    if (!bRet) {
        std::ostringstream stream;
        stream << " SendMsgWithOutHead failed fd " << sockIndex;
        stream << " msg id " << msgID;
        m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
    }

    return bRet;
}

bool NetModule::SendMsgToAllClientWithOutHead(const int msgID, const std::string &msg) {
    bool bRet = m_pNet->SendMsgToAllClientWithOutHead(msgID, msg.c_str(), (uint32_t)msg.length());
    if (!bRet) {
        std::ostringstream stream;
        stream << " SendMsgToAllClientWithOutHead failed";
        stream << " msg id " << msgID;
        m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
    }

    return bRet;
}

bool NetModule::SendMsgPB(const uint16_t msgID, const google::protobuf::Message &xData, const SQUICK_SOCKET sockIndex, const Guid id) {
    SquickStruct::MsgBase xMsg;
    if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
        std::ostringstream stream;
        stream << " SendMsgPB Message to  " << sockIndex;
        stream << " Failed For Serialize of MsgData, MessageID " << msgID;
        m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    SquickStruct::Ident *pPlayerID = xMsg.mutable_player_id();
    *pPlayerID = StructToProtobuf(id);

    std::string msg;
    if (!xMsg.SerializeToString(&msg)) {
        std::ostringstream stream;
        stream << " SendMsgPB Message to  " << sockIndex;
        stream << " Failed For Serialize of MsgBase, MessageID " << msgID;
        m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    return SendMsgWithOutHead(msgID, msg, sockIndex);
}

bool NetModule::SendMsg(const uint16_t msgID, const std::string &xData, const SQUICK_SOCKET sockIndex) { return SendMsgWithOutHead(msgID, xData, sockIndex); }

bool NetModule::SendMsg(const uint16_t msgID, const std::string &xData, const SQUICK_SOCKET sockIndex, const Guid id) {
    SquickStruct::MsgBase xMsg;
    xMsg.set_msg_data(xData.data(), xData.length());

    SquickStruct::Ident *pPlayerID = xMsg.mutable_player_id();
    *pPlayerID = StructToProtobuf(id);

    std::string msg;
    if (!xMsg.SerializeToString(&msg)) {
        std::ostringstream stream;
        stream << " SendMsgPB Message to  " << sockIndex;
        stream << " Failed For Serialize of MsgBase, MessageID " << msgID;
        m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    return SendMsgWithOutHead(msgID, msg, sockIndex);
}

bool NetModule::SendMsgPB(const uint16_t msgID, const google::protobuf::Message &xData, const SQUICK_SOCKET sockIndex) {
    SquickStruct::MsgBase xMsg;
    if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
        std::ostringstream stream;
        stream << " SendMsgPB Message to  " << sockIndex;
        stream << " Failed For Serialize of MsgData, MessageID " << msgID;
        m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    SquickStruct::Ident *pPlayerID = xMsg.mutable_player_id();
    *pPlayerID = StructToProtobuf(Guid());

    std::string msg;
    if (!xMsg.SerializeToString(&msg)) {
        std::ostringstream stream;
        stream << " SendMsgPB Message to  " << sockIndex;
        stream << " Failed For Serialize of MsgBase, MessageID " << msgID;
        m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    SendMsgWithOutHead(msgID, msg, sockIndex);

    return true;
}

bool NetModule::SendMsgPBToAllClient(const uint16_t msgID, const google::protobuf::Message &xData) {
    SquickStruct::MsgBase xMsg;
    if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
        std::ostringstream stream;
        stream << " SendMsgPBToAllClient";
        stream << " Failed For Serialize of MsgData, MessageID " << msgID;
        m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    std::string msg;
    if (!xMsg.SerializeToString(&msg)) {
        std::ostringstream stream;
        stream << " SendMsgPBToAllClient";
        stream << " Failed For Serialize of MsgBase, MessageID " << msgID;
        m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    return SendMsgToAllClientWithOutHead(msgID, msg);
}

bool NetModule::SendMsgPB(const uint16_t msgID, const google::protobuf::Message &xData, const SQUICK_SOCKET sockIndex, const std::vector<Guid> *pClientIDList) {
    if (!m_pNet) {
        std::ostringstream stream;
        stream << " m_pNet SendMsgPB faailed fd " << sockIndex;
        stream << " Failed For Serialize of MsgBase, MessageID " << msgID;
        m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    SquickStruct::MsgBase xMsg;
    if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
        std::ostringstream stream;
        stream << " SendMsgPB faailed fd " << sockIndex;
        stream << " Failed For Serialize of MsgBase, MessageID " << msgID;
        m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    SquickStruct::Ident *pPlayerID = xMsg.mutable_player_id();
    *pPlayerID = StructToProtobuf(Guid());
    if (pClientIDList) {
        for (int i = 0; i < pClientIDList->size(); ++i) {
            const Guid &ClientID = (*pClientIDList)[i];

            SquickStruct::Ident *pData = xMsg.add_player_client_list();
            if (pData) {
                *pData = StructToProtobuf(ClientID);
            }
        }
    }

    std::string msg;
    if (!xMsg.SerializeToString(&msg)) {
        std::ostringstream stream;
        stream << " SendMsgPB faailed fd " << sockIndex;
        stream << " Failed For Serialize of MsgBase, MessageID " << msgID;
        m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    return SendMsgWithOutHead(msgID, msg, sockIndex);
}

bool NetModule::SendMsgPB(const uint16_t msgID, const std::string &strData, const SQUICK_SOCKET sockIndex, const std::vector<Guid> *pClientIDList) {
    if (!m_pNet) {
        std::ostringstream stream;
        stream << " SendMsgPB NULL Of Net faailed fd " << sockIndex;
        stream << " Failed For Serialize of MsgBase, MessageID " << msgID;
        m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    SquickStruct::MsgBase xMsg;
    xMsg.set_msg_data(strData.data(), strData.length());

    SquickStruct::Ident *pPlayerID = xMsg.mutable_player_id();
    *pPlayerID = StructToProtobuf(Guid());
    if (pClientIDList) {
        for (int i = 0; i < pClientIDList->size(); ++i) {
            const Guid &ClientID = (*pClientIDList)[i];

            SquickStruct::Ident *pData = xMsg.add_player_client_list();
            if (pData) {
                *pData = StructToProtobuf(ClientID);
            }
        }
    }

    std::string msg;
    if (!xMsg.SerializeToString(&msg)) {
        std::ostringstream stream;
        stream << " SendMsgPB failed fd " << sockIndex;
        stream << " Failed For Serialize of MsgBase, MessageID " << msgID;
        m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    return SendMsgWithOutHead(msgID, msg, sockIndex);
}

INet *NetModule::GetNet() { return m_pNet; }

void NetModule::OnReceiveNetPack(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    // m_pLogModule->LogInfo(pPluginManager->GetAppName() + std::to_string(pPluginManager->GetAppID()) + " NetModule::OnReceiveNetPack " +
    // std::to_string(msgID), __FILE__, __LINE__);

    Performance performance;

#if SQUICK_PLATFORM != SQUICK_PLATFORM_WIN
    SQUICK_CRASH_TRY
#endif

    std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::iterator it = mxReceiveCallBack.find(msgID);
    if (mxReceiveCallBack.end() != it) {
        std::list<NET_RECEIVE_FUNCTOR_PTR> &xFunList = it->second;
        for (std::list<NET_RECEIVE_FUNCTOR_PTR>::iterator itList = xFunList.begin(); itList != xFunList.end(); ++itList) {
            NET_RECEIVE_FUNCTOR_PTR &pFunPtr = *itList;
            NET_RECEIVE_FUNCTOR *pFunc = pFunPtr.get();

            pFunc->operator()(sockIndex, msgID, msg, len);
        }
    } else {
        for (std::list<NET_RECEIVE_FUNCTOR_PTR>::iterator itList = mxCallBackList.begin(); itList != mxCallBackList.end(); ++itList) {
            NET_RECEIVE_FUNCTOR_PTR &pFunPtr = *itList;
            NET_RECEIVE_FUNCTOR *pFunc = pFunPtr.get();

            pFunc->operator()(sockIndex, msgID, msg, len);
        }
    }

#if SQUICK_PLATFORM != SQUICK_PLATFORM_WIN
    SQUICK_CRASH_END
#endif
    /*
            if (performance.CheckTimePoint(5))
            {
                    std::ostringstream os;
                    os << "---------------net module performance problem------------------- ";
                    os << performance.TimeScope();
                    os << "---------- MsgID: ";
                    os << msgID;
                    m_pLogModule->LogWarning(Guid(0, msgID), os, __FUNCTION__, __LINE__);
            }
     */
}

void NetModule::OnSocketNetEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    for (std::list<NET_EVENT_FUNCTOR_PTR>::iterator it = mxEventCallBackList.begin(); it != mxEventCallBackList.end(); ++it) {
        NET_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        NET_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(sockIndex, eEvent, pNet);
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

    SquickStruct::ServerHeartBeat xMsg;
    xMsg.set_count(0);

    SendMsgPB(SquickStruct::ServerRPC::STS_HEART_BEAT, xMsg, 0);
}