
#include "ws_module.h"
#include "i_net.h"
#include "third_party/common/base64.hpp"
#include "third_party/common/http_util.hpp"
#include "third_party/common/sha1.hpp"
#include "ws_error.h"

enum ws_connection_state {
    ws_init = 0,
    ws_handshaked,
};

enum class opcode : std::uint8_t {
    incomplete = 0,
    text = 1,
    binary = 2,
    rsv3 = 3,
    rsv4 = 4,
    rsv5 = 5,
    rsv6 = 6,
    rsv7 = 7,
    close = 8,
    ping = 9,
    pong = 10,
    crsvb = 11,
    crsvc = 12,
    crsvd = 13,
    crsve = 14,
    crsvf = 15
};

struct frame_header {
    bool fin;
    bool rsv1;
    bool rsv2;
    bool rsv3;
    bool mask;
    opcode op;           // 4bit
    uint8_t payload_len; // 7 bit
    std::uint32_t key;
};

static constexpr size_t HANDSHAKE_MAX_SIZE = 8192;

static constexpr size_t PAYLOAD_MIN_LEN = 125;
static constexpr size_t PAYLOAD_MID_LEN = 126;
static constexpr size_t PAYLOAD_MAX_LEN = 127;
static constexpr size_t FIN_FRAME_FLAG = 0x80; // 1 0 0 0 0 0 0 0
#define MAX_PACKAGE_SIZE 8096

WSModule::WSModule(IPluginManager *p) {
    is_update_ = true;
    pm_ = p;

    mnBufferSize = 0;
    mLastTime = GetPluginManager()->GetNowTime();
    m_pNet = NULL;
}

WSModule::~WSModule() {
    if (m_pNet) {
        m_pNet->Final();
    }

    delete m_pNet;
    m_pNet = NULL;
}

bool WSModule::Start() {
    m_log_ = pm_->FindModule<ILogModule>();

    return true;
}

bool WSModule::AfterStart() { return true; }

void WSModule::Connect(const char *ip, const unsigned short nPort, const uint32_t expand_buffer_size) {
    m_pNet = new Net(this, &WSModule::OnReceiveNetPack, &WSModule::OnSocketNetEvent, true);
    m_pNet->Connect(ip, nPort, expand_buffer_size);
}

int WSModule::Listen(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount, const uint32_t expand_buffer_size) {
    m_pNet = new Net(this, &WSModule::OnReceiveNetPack, &WSModule::OnSocketNetEvent, true);
    return m_pNet->Listen(nMaxClient, nPort, nCpuCount, expand_buffer_size);
}


void WSModule::RemoveReceiveCallBack(const int msg_id) {
    std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::iterator it = mxReceiveCallBack.find(msg_id);
    if (mxReceiveCallBack.end() != it) {
        mxReceiveCallBack.erase(it);
    }
}

bool WSModule::AddReceiveCallBack(const int msg_id, const NET_RECEIVE_FUNCTOR_PTR &cb) {
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

bool WSModule::AddReceiveCallBack(const NET_RECEIVE_FUNCTOR_PTR &cb) {
    mxCallBackList.push_back(cb);

    return true;
}

bool WSModule::AddEventCallBack(const NET_EVENT_FUNCTOR_PTR &cb) {
    mxEventCallBackList.push_back(cb);

    return true;
}

bool WSModule::Update() {
    if (!m_pNet) {
        return false;
    }

    KeepAlive();

    return m_pNet->Update();
}

bool WSModule::SendPBMsg(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock) {
    std::string msg;
    if (!xData.SerializeToString(&msg)) {
        LOG_ERROR("SendPBMsg failed serialize to string, msg_id<%v>", msg_id);
        return false;
    }
    
    return SendMsg(msg_id, msg.c_str(), msg.length(), sock);
}

bool WSModule::SendMsg(const int16_t msg_id, const char *msg, const size_t len, const socket_t sock /*= 0*/) {
    std::string strOutData;
    int nAllLen = EnCode(msg_id, msg, len, strOutData);
    if (nAllLen == len + IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH) {
        auto frame = EncodeFrame(strOutData.data(), strOutData.size(), false);
        return SendRawMsg(frame, sock);
    }

    return false;
}

int WSModule::EnCode(const uint16_t umsg_id, const char *strData, const uint32_t unDataLen, std::string &strOutData) {
    rpcHead xHead;
    xHead.SetMsgID(umsg_id);
    xHead.SetBodyLength(unDataLen);

    char szHead[IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH] = {0};
    xHead.EnCode(szHead);

    strOutData.clear();
    strOutData.append(szHead, IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH);
    strOutData.append(strData, unDataLen);

    return xHead.GetBodyLength() + IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH;
}

bool WSModule::SendData(const std::string &msg, const socket_t sock, const bool text) {
    auto frame = EncodeFrame(msg.data(), msg.size(), text);
    return SendRawMsg(frame, sock);
}

bool WSModule::SendDataToAllClient(const std::string &msg, const bool text) {
    auto frame = EncodeFrame(msg.data(), msg.size(), text);
    bool bRet = m_pNet->SendDataToAllClient(frame.c_str(), (uint32_t)frame.length());
    if (!bRet) {
        LOG_ERROR("SendDataToAllClient failed");
    }

    return bRet;
}

INet *WSModule::GetNet() { return m_pNet; }

void WSModule::OnError(const socket_t sock, const std::error_code &e) {
    // may write/print error log
    // then close socket
#if PLATFORM != PLATFORM_WIN
    SQUICK_CRASH_TRY
#endif
    for (auto &cb : mxEventCallBackList) {
        NET_EVENT_FUNCTOR_PTR &pFunPtr = cb;
        NET_EVENT_FUNCTOR *pFunc = pFunPtr.get();

        pFunc->operator()(sock, SQUICK_NET_EVENT::SQUICK_NET_EVENT_ERROR, m_pNet);
    }

#if PLATFORM != PLATFORM_WIN
    SQUICK_CRASH_END
#endif

    LOG_ERROR("WebSocket error: <%v>, <%v>", e.value(), e.message());
    m_pNet->CloseNetObject(sock);
}

bool WSModule::SendRawMsg(const std::string &msg, const socket_t sock) {
    bool bRet = m_pNet->SendData(msg.c_str(), (uint32_t)msg.length(), sock);
    if (!bRet) {
        LOG_ERROR("SendRawMsg failed sock<%v>", sock);
    }

    return bRet;
}

void WSModule::OnReceiveNetPack(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    if (msg_id < 0) {
        NetObject *pNetObject = m_pNet->GetNetObject(sock);
        if (nullptr != pNetObject) {
            switch (pNetObject->GetConnectState()) {
            case ws_init: {
                std::string_view data(pNetObject->GetBuff(), pNetObject->GetBuffLen());
                auto pos = data.find("\r\n\r\n");
                if (pos != std::string_view::npos) {
                    auto ec = HandShake(sock, data.data(), pos);
                    if (ec) {
                        // mark need send then close here:
                        SendRawMsg("HTTP/1.1 400 Bad Request\r\n\r\n", sock);
                        // log ec.message()
                        // OnError(sock, ec);
                        return;
                    }
                    pNetObject->RemoveBuff(0, pos + 4);
                    pNetObject->SetConnectState(ws_handshaked);
                    // may have more data, check it
                    ec = DecodeFrame(sock, pNetObject);
                    if (ec) {
                        OnError(sock, ec);
                        return;
                    }
                } else if (data.size() > HANDSHAKE_MAX_SIZE) {
                    OnError(sock, websocket::make_error_code(websocket::error::buffer_overflow));
                    return;
                }
                break;
            }
            case ws_handshaked: {
                auto ec = DecodeFrame(sock, pNetObject);
                if (ec) {
                    OnError(sock, ec);
                    return;
                }
                break;
            }
            default:
                break;
            }
        }
    } else {
        LOG_INFO("OnReceiveNetPack msg_id<%v>", msg_id);
#if PLATFORM != PLATFORM_WIN
        SQUICK_CRASH_TRY
#endif
        auto it = mxReceiveCallBack.find(msg_id);
        if (mxReceiveCallBack.end() != it) {
            auto &xFunList = it->second;
            for (auto itList = xFunList.begin(); itList != xFunList.end(); ++itList) {
                auto &pFunPtr = *itList;
                auto pFunc = pFunPtr.get();

                pFunc->operator()(sock, msg_id, msg, len);
            }
        } else {
            for (auto itList = mxCallBackList.begin(); itList != mxCallBackList.end(); ++itList) {
                auto &pFunPtr = *itList;
                auto pFunc = pFunPtr.get();

                pFunc->operator()(sock, msg_id, msg, len);
            }
        }
#if PLATFORM != PLATFORM_WIN
        SQUICK_CRASH_END
#endif
    }
}

void WSModule::OnSocketNetEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    for (auto it = mxEventCallBackList.begin(); it != mxEventCallBackList.end(); ++it) {
        auto &pFunPtr = *it;
        auto pFunc = pFunPtr.get();
        pFunc->operator()(sock, eEvent, pNet);
    }
}

void WSModule::KeepAlive() {
    if (!m_pNet) {
        return;
    }

    if (m_pNet->IsServer()) {
        return;
    }

    if (mLastTime + 10 > GetPluginManager()->GetNowTime()) {
        return;
    }

    mLastTime = GetPluginManager()->GetNowTime();
}

std::error_code WSModule::HandShake(const socket_t sock, const char *msg, const uint32_t len) {
    std::string_view data{msg, len};
    std::string_view method;
    std::string_view ignore;
    std::string_view version;
    http::util::case_insensitive_multimap_view header;
    if (!http::util::request_parser::parse(data, method, ignore, ignore, version, header)) {
        return websocket::make_error_code(websocket::error::ws_bad_http_header);
    }

    if (version < "1.0" || version > "1.1") {
        return make_error_code(websocket::error::ws_bad_http_version);
    }

    if (method != "GET")
        return make_error_code(websocket::error::ws_bad_method);

    std::string_view connection;
    if (!http::util::try_get_header(header, "connection", connection))
        return make_error_code(websocket::error::ws_no_connection);

    std::string_view upgrade;
    if (!http::util::try_get_header(header, "upgrade", upgrade))
        return make_error_code(websocket::error::ws_no_upgrade);

    if (!http::util::iequal_string(connection, std::string_view{"upgrade"}))
        return make_error_code(websocket::error::ws_no_connection_upgrade);

    if (!http::util::iequal_string(upgrade, std::string_view{"websocket"}))
        return make_error_code(websocket::error::ws_no_upgrade_websocket);

    std::string_view sec_ws_key;
    if (!http::util::try_get_header(header, std::string_view{"sec-websocket-key"}, sec_ws_key))
        return make_error_code(websocket::error::ws_no_sec_key);

    if (base64_decode(std::string{sec_ws_key.data(), sec_ws_key.size()}).size() != 16)
        return make_error_code(websocket::error::ws_bad_sec_key);

    std::string_view sec_ws_version;
    if (!http::util::try_get_header(header, "sec-websocket-version", sec_ws_version))
        return make_error_code(websocket::error::ws_no_sec_version);

    if (sec_ws_version != "13") {
        return make_error_code(websocket::error::ws_bad_sec_version);
    }

    std::string_view protocol;
    http::util::try_get_header(header, "sec-websocket-protocol", protocol);

    std::string response;
    response.append("HTTP/1.1 101 Switching Protocols\r\n");
    response.append("Upgrade: WebSocket\r\n");
    response.append("Connection: Upgrade\r\n");
    response.append("Sec-WebSocket-Accept: ");
    response.append(HashKey(sec_ws_key.data(), sec_ws_key.size()));
    response.append("\r\n", 2);
    if (!protocol.empty()) {
        response.append("Sec-WebSocket-Protocol: ");
        response.append(protocol.data(), protocol.size());
        response.append("\r\n", 2);
    }
    response.append("\r\n", 2);
    SendRawMsg(response, sock);

    return std::error_code();
}

std::error_code WSModule::DecodeFrame(const socket_t sock, NetObject *pNetObject) {
    const char *data = pNetObject->GetBuff();
    size_t size = pNetObject->GetBuffLen();
    const uint8_t *tmp = (const uint8_t *)(data);

    if (size < 3) {
        return std::error_code();
    }

    size_t need = 2;
    frame_header fh;

    fh.payload_len = tmp[1] & 0x7F;
    switch (fh.payload_len) {
    case PAYLOAD_MID_LEN:
        need += 2;
        break;
    case PAYLOAD_MAX_LEN:
        need += 8;
        break;
    default:
        break;
    }

    fh.mask = (tmp[1] & 0x80) != 0;
    // message client to server must masked.
    if (!fh.mask) {
        return make_error_code(websocket::error::ws_bad_unmasked_frame);
    }

    if (fh.mask) {
        need += 4;
    }

    // need more data
    if (size < need) {
        return std::error_code();
    }

    fh.op = static_cast<opcode>(tmp[0] & 0x0F);
    fh.fin = (tmp[0] & 0x80) != 0;
    fh.rsv1 = (tmp[0] & 0x40) != 0;
    fh.rsv2 = (tmp[0] & 0x20) != 0;
    fh.rsv3 = (tmp[0] & 0x10) != 0;

    switch (fh.op) {
    case opcode::text:
    case opcode::binary:
        if (fh.rsv1 || fh.rsv2 || fh.rsv3) {
            // reserved bits not cleared
            return make_error_code(websocket::error::ws_bad_reserved_bits);
        }
        break;
    case opcode::incomplete: {
        // not support continuation frame
        return make_error_code(websocket::error::ws_bad_continuation);
        break;
    }
    default:
        if (!fh.fin) {
            // not support fragmented control message
            return make_error_code(websocket::error::ws_bad_control_fragment);
        }
        if (fh.payload_len > PAYLOAD_MIN_LEN) {
            // invalid length for control message
            return make_error_code(websocket::error::ws_bad_control_size);
        }
        if (fh.rsv1 || fh.rsv2 || fh.rsv3) {
            // reserved bits not cleared
            return make_error_code(websocket::error::ws_bad_reserved_bits);
        }
        break;
    }

    uint64_t reallen = 0;
    switch (fh.payload_len) {
    case PAYLOAD_MID_LEN: {
        auto n = *(uint16_t *)(&tmp[2]);
        reallen = IMsgHead::SQUICK_NTOHS(n);
        if (reallen < PAYLOAD_MID_LEN) {
            // length not canonical
            return make_error_code(websocket::error::ws_bad_size);
        }
        break;
    }
    case PAYLOAD_MAX_LEN: {
        // unsupport 64bit len data frame
        // game server 64K is enough for client to server
        return make_error_code(websocket::error::ws_bad_size);
        // reallen = *(uint64_t*)(&tmp[2]);
        // reallen = IMsgHead::SQUICK_NTOHLL(reallen);
        // if (reallen < 65536)
        // {
        //     // length not canonical
        //     return make_error_code(websocket::error::ws_bad_size);
        // }
        // break;
    }
    default:
        reallen = fh.payload_len;
        break;
    }

    if (reallen > MAX_PACKAGE_SIZE) {
        return make_error_code(websocket::error::ws_bad_size);
    }

    if (size < need + reallen) {
        // need more data
        return std::error_code();
    }

    if (fh.mask) {
        fh.key = *((uint32_t *)(tmp + (need - sizeof(fh.key))));
        // unmask data:
        uint8_t *d = (uint8_t *)(tmp + need);
        for (uint64_t i = 0; i < reallen; i++) {
            d[i] = d[i] ^ ((uint8_t *)(&fh.key))[i % 4];
        }
    }

    if (fh.op == opcode::close) {
        // mark: may have error msg
        return websocket::make_error_code(websocket::error::ws_closed);
    }

    // mark:
    // need : control frame len
    // reallen : payload  len
    // write on message callback here
    // callback(data+need,reallen)

    if (fh.op == opcode::binary) {
        const char *pbData = data + need;
        rpcHead xHead;
        int nMsgBodyLength = DeCode(pbData, reallen, xHead);
        if (nMsgBodyLength >= 0 && xHead.GetMsgID() > 0) {
            OnReceiveNetPack(sock, xHead.GetMsgID(), pbData + IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH, nMsgBodyLength);
        }
        else {
            LOG_ERROR("No this msg_id: %v", nMsgBodyLength);
        }
    } else if (fh.op == opcode::text) {
        const char *pbData = data + need;
        OnReceiveNetPack(sock, 0, pbData, reallen);
    }

    // remove control frame
    size_t offset = need + reallen;
    pNetObject->RemoveBuff(0, offset);

    return DecodeFrame(sock, pNetObject);
}

int WSModule::DeCode(const char *strData, const uint32_t unAllLen, rpcHead &xHead) {
    if (unAllLen < IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH) {
        return -1;
    }
    if (IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH != xHead.DeCode(strData)) {
        return -2;
    }
    if (xHead.GetBodyLength() > (unAllLen - IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH)) {
        return -3;
    }

    return xHead.GetBodyLength();
}

std::string WSModule::EncodeFrame(const char *data, size_t size_, bool text) {
    // may write a buffer with headreserved space
    std::string res;
    res.reserve(size_ + 10);

    std::string sizebuf;
    uint64_t size = size_;

    uint8_t payload_len = 0;
    if (size <= PAYLOAD_MIN_LEN) {
        payload_len = static_cast<uint8_t>(size);
    } else if (size <= UINT16_MAX) {
        payload_len = static_cast<uint8_t>(PAYLOAD_MID_LEN);
        uint16_t n = (uint16_t)size;
        n = IMsgHead::SQUICK_HTONS(n);
        sizebuf.append(reinterpret_cast<const char *>(&n), sizeof(n));
    } else {
        payload_len = static_cast<uint8_t>(PAYLOAD_MAX_LEN);
        size = IMsgHead::SQUICK_HTONLL(size);
        sizebuf.append(reinterpret_cast<const char *>(&size), sizeof(size));
    }

    uint8_t ocode = FIN_FRAME_FLAG | static_cast<uint8_t>(opcode::binary);
    if (text) {
        ocode = FIN_FRAME_FLAG | static_cast<uint8_t>(opcode::text);
    }

    res.append(reinterpret_cast<const char *>(&ocode), sizeof(opcode));
    res.append(reinterpret_cast<const char *>(&payload_len), sizeof(payload_len));
    if (!sizebuf.empty())
        res.append(sizebuf);

    res.append(data, size);
    return res;
}

std::string WSModule::HashKey(const char *key, size_t len) {
    uint8_t keybuf[60] = {0};
    std::memcpy(keybuf, key, len);
    std::memcpy(keybuf + len, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36);

    uint8_t shakey[sha1::sha1_context::digest_size] = {0};
    sha1::sha1_context ctx;
    sha1::init(ctx);
    sha1::update(ctx, keybuf, sizeof(keybuf));
    sha1::finish(ctx, shakey);
    return base64_encode(shakey, sizeof(shakey));
}
