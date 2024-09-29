#pragma once

#include "i_pb_log_module.h"
#include "i_ws_module.h"
#include "net_module.h"
#include <iostream>

class WSModule : public IWSModule {
  public:
    WSModule(IPluginManager *p);

    virtual ~WSModule();

    virtual bool Start();
    virtual bool AfterStart();

    // as client
    virtual void Connect(const char *ip, const unsigned short nPort, const uint32_t expand_buffer_size);
    // as server
    virtual int Listen(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount, const uint32_t expand_buffer_size);
    virtual void RemoveReceiveCallBack(const int msg_id);
    virtual bool AddReceiveCallBack(const int msg_id, const NET_RECEIVE_FUNCTOR_PTR &cb);
    virtual bool AddReceiveCallBack(const NET_RECEIVE_FUNCTOR_PTR &cb);
    virtual bool AddEventCallBack(const NET_EVENT_FUNCTOR_PTR &cb);

    virtual bool Update();

    virtual bool SendData(const std::string &msg, const socket_t sock, const bool text = true) override;
    virtual bool SendDataToAllClient(const std::string &msg, const bool text = true) override;
    virtual bool SendMsg(const int16_t msg_id, const char *msg, const size_t len, const socket_t sock /*= 0*/) override;
    virtual bool SendPBMsg(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock) override;
    int EnCode(const uint16_t umsg_id, const char *strData, const uint32_t unDataLen, std::string &strOutData);
    virtual INet *GetNet();

    virtual void OnError(const socket_t sock, const std::error_code &e);

  protected:
    bool SendRawMsg(const std::string &msg, const socket_t sock);
    void OnReceiveNetPack(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnSocketNetEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);
    void KeepAlive();

    std::error_code HandShake(const socket_t sock, const char *msg, const uint32_t len);
    std::error_code DecodeFrame(const socket_t sock, NetObject *pNetObject);
    int DeCode(const char *strData, const uint32_t unAllLen, rpcHead &xHead);

    std::string EncodeFrame(const char *data, size_t size, bool text);
    std::string HashKey(const char *key, size_t len);

  private:
    unsigned int mnBufferSize;
    INet *m_pNet;
    INT64 mLastTime;
    std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>> mxReceiveCallBack;
    std::list<NET_EVENT_FUNCTOR_PTR> mxEventCallBackList;
    std::list<NET_RECEIVE_FUNCTOR_PTR> mxCallBackList;

    ILogModule *m_log_;
    IPbLogModule *m_pb_log_;
};