
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
    m_pb_log_ = pm_->FindModule<IPbLogModule>();
    return true;
}

bool NetModule::AfterStart() { return true; }

void NetModule::Connect(const char *ip, const unsigned short nPort, const uint32_t expand_buffer_size) {
    m_pNet = new Net(this, &NetModule::OnReceiveNetPack, &NetModule::OnSocketNetEvent);
    m_pNet->Connect(ip, nPort, expand_buffer_size);
}

int NetModule::Listen(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount, const uint32_t expand_buffer_size) {
    m_pNet = new Net(this, &NetModule::OnReceiveNetPack, &NetModule::OnSocketNetEvent);
    return m_pNet->Listen(nMaxClient, nPort, nCpuCount, expand_buffer_size);
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

bool NetModule::AddReceiveCallBack(const int msg_id, const NET_CORO_RECEIVE_FUNCTOR_PTR &cb) {
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

bool NetModule::SendMsg(const int msg_id, const std::string &msg, const socket_t sock) {
    bool bRet = m_pNet->SendMsg(msg_id, msg.c_str(), (uint32_t)msg.length(), sock);
    if (!bRet) {
        LOG_ERROR("Failed to send msg msg_id <%v> ", msg_id);
    }
#ifdef SQUICK_DEV
    m_pb_log_->Log("Send Msg by m_net: ", msg_id, msg.data(), msg.length());
#endif
    return bRet;
}

bool NetModule::SendMsgToAllClient(const int msg_id, const std::string &msg) {
    bool bRet = m_pNet->SendMsgToAllClient(msg_id, msg.c_str(), (uint32_t)msg.length());
    if (!bRet) {
        LOG_ERROR("Failed to send msg msg_id <%v> ", msg_id);
    }

    return bRet;
}

bool NetModule::SendPBToNode(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock, const uint64_t uid, reqid_t req_id) {
    rpc::MsgBase xMsg;
    if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
        std::ostringstream stream;
        LOG_ERROR("Failed for serialize of protobuf, msg_id <%v> ", sock, msg_id);
        return false;
    }

    xMsg.set_uid(uid);
    xMsg.set_req_id(req_id);
    xMsg.set_id(pm_->GetAppID());
    std::string msg;
    if (!xMsg.SerializeToString(&msg)) {
        LOG_ERROR("Failed for serialize of protobuf, msg_id <%v> ", sock, msg_id);
        return false;
    }

    return SendMsg(msg_id, msg, sock);
}

bool NetModule::SendToNode(const uint16_t msg_id, const std::string &xData, const socket_t sock, const uint64_t uid, reqid_t req_id) {
    rpc::MsgBase xMsg;
    xMsg.set_msg_data(xData.data(), xData.length());
    xMsg.set_uid(uid);
    xMsg.set_req_id(req_id);
    xMsg.set_id(pm_->GetAppID());
    std::string msg;
    if (!xMsg.SerializeToString(&msg)) {
        LOG_ERROR("Failed for serialize of protobuf, msg_id <%v> ", sock, msg_id);
        return false;
    }
    return SendMsg(msg_id, msg, sock);
}

bool NetModule::SendPBToAllNodeClient(const uint16_t msg_id, const google::protobuf::Message &xData) {
    rpc::MsgBase xMsg;
    xMsg.set_id(pm_->GetAppID());
    if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
        LOG_ERROR("Failed for serialize of protobuf, msg_id <%v> ", msg_id);
        return false;
    }

    std::string msg;
    if (!xMsg.SerializeToString(&msg)) {
        LOG_ERROR("Failed for serialize of protobuf, msg_id <%v> ", msg_id);
        return false;
    }

    return SendMsgToAllClient(msg_id, msg);
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
    // m_log_->LogInfo(pm_->GetAppName() + std::to_string(pm_->GetAppID()) + " NetModule::OnReceiveNetPack " +
    // std::to_string(msg_id), __FILE__, __LINE__);

    Performance performance;

#if PLATFORM != PLATFORM_WIN
    SQUICK_CRASH_TRY
#endif
    // Log PB
#ifdef SQUICK_DEV
    m_pb_log_->Log("Received Msg by m_net: ", msg_id, msg, len);
#endif

    // corotine first handle
    auto co_it = coro_funcs_.find(msg_id);
    if (coro_funcs_.end() != co_it) {
        std::list<NET_CORO_RECEIVE_FUNCTOR_PTR> &funcs = co_it->second;
        for (auto func_iter = funcs.begin(); func_iter != funcs.end(); ++func_iter) {
            NET_CORO_RECEIVE_FUNCTOR_PTR &pFunPtr = *func_iter;
            NET_CORO_RECEIVE_FUNCTOR *pFunc = pFunPtr.get();
            auto co = pFunc->operator()(sock, msg_id, msg, len);
            coroutines_.push_back(co);
#ifdef SQUICK_DEV
            LOG_DEBUG("Net Module create a new coroutine: %v", co.GetHandle().address());
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

    if (performance.CheckTimePoint(10)) {
        LOG_WARN("Net handle time grate than %v, msg_id<%v>, cost time<%v>", 10, msg_id, performance.TimeScope());
    }

    SetSquickMainThreadSleep(false);
}

void NetModule::OnSocketNetEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    for (std::list<NET_EVENT_FUNCTOR_PTR>::iterator it = mxEventCallBackList.begin(); it != mxEventCallBackList.end(); ++it) {
        NET_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        NET_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(sock, eEvent, pNet);
    }
    SetSquickMainThreadSleep(false);
}
