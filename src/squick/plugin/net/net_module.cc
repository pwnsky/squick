
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

bool NetModule::AddReceiveCallBack(const int msg_id, const NET_CORO_RECEIVE_FUNCTOR_PTR& cb) {
    if (coro_funcs_.find(msg_id) == coro_funcs_.end()) {
        std::list<NET_CORO_RECEIVE_FUNCTOR_PTR> xList;
        xList.push_back(cb);
        coro_funcs_.insert(std::map<int, std::list<NET_CORO_RECEIVE_FUNCTOR_PTR>>::value_type(msg_id, xList));
        return true;
    }

    std::map<int, std::list<NET_CORO_RECEIVE_FUNCTOR_PTR>>::iterator it = coro_funcs_.find(msg_id);
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

    m_pNet->Update();

    time_t now_time = time(nullptr);
    if (now_time - last_check_coroutines_time_ > 0) {
        FixCoroutines(now_time);
        last_check_coroutines_time_ = now_time;
    }

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

bool NetModule::SendMsgPB(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock, const string guid, reqid_t req_id) {
    rpc::MsgBase xMsg;
    if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
        std::ostringstream stream;
        stream << " SendMsgPB Message to  " << sock;
        stream << " Failed For Serialize of MsgData, MessageID " << msg_id;
        m_log_->LogError(stream, __FUNCTION__, __LINE__);

        return false;
    }

    xMsg.set_guid(guid);
    xMsg.set_req_id(req_id);
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

bool NetModule::SendMsg(const uint16_t msg_id, const std::string &xData, const socket_t sock, const string guid, reqid_t req_id) {
    rpc::MsgBase xMsg;
    xMsg.set_msg_data(xData.data(), xData.length());
    xMsg.set_guid(guid);
    xMsg.set_req_id(req_id);
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

    xMsg.set_guid(Guid(pm_->GetAppID(), 0).ToString());
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

INet *NetModule::GetNet() { return m_pNet; }

int NetModule::FixCoroutines(time_t now_time) {
    // Check coroutine states and free coroutines objects
    int num = 0;
    for (auto iter = coroutines_.begin(); iter != coroutines_.end();) {
        auto now_iter = iter;
        ++iter;
        auto co = *now_iter;
        if (co.GetHandle().done()) {
            co.GetHandle().destroy();
#ifdef SQUICK_DEV
            dout << "Destoy coroutine: " << co.GetHandle().address() << endl;
#endif 
            coroutines_.erase(now_iter);
            num++;
            continue;
        }

        if (now_time - co.GetStartTime() > NET_COROTINE_MAX_SURVIVAL_TIME) {
#ifdef SQUICK_DEV
            dout << " This corotine has time out: " << co.GetHandle().address() << std::endl;
#endif
            // do not destroy 
            if (!co.GetHandle().done()) {
                co.GetHandle().resume();
            }
        }
    }

    return num;
}

void NetModule::OnReceiveNetPack(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    //m_log_->LogInfo(pm_->GetAppName() + std::to_string(pm_->GetAppID()) + " NetModule::OnReceiveNetPack " +
    //std::to_string(msg_id), __FILE__, __LINE__);

    Performance performance;

#if PLATFORM != PLATFORM_WIN
    SQUICK_CRASH_TRY
#endif

    // corotine first handle

    auto co_it = coro_funcs_.find(msg_id);
    if (coro_funcs_.end() != co_it) {
        std::list<NET_CORO_RECEIVE_FUNCTOR_PTR>& funcs = co_it->second;
        for (auto func_iter = funcs.begin(); func_iter != funcs.end(); ++func_iter) {
            NET_CORO_RECEIVE_FUNCTOR_PTR& pFunPtr = *func_iter;
            NET_CORO_RECEIVE_FUNCTOR* pFunc = pFunPtr.get();
            auto co = pFunc->operator()(sock, msg_id, msg, len);
            coroutines_.push_back(co);
#ifdef SQUICK_DEV
            dout << "Net Module create a new coroutine: " << co.GetHandle().address() << endl;
#endif
            co.GetHandle().resume();
        }
    }

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
