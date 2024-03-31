#pragma once

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
    virtual void Startialization(const char *ip, const unsigned short nPort);

    // as server
    virtual int Startialization(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount = 4);

    virtual unsigned int ExpandBufferSize(const unsigned int size = 1024 * 1024 * 20) override;

    virtual void RemoveReceiveCallBack(const int msg_id);

    virtual bool AddReceiveCallBack(const int msg_id, const NET_RECEIVE_FUNCTOR_PTR &cb);

    virtual bool AddReceiveCallBack(const NET_RECEIVE_FUNCTOR_PTR &cb);

    virtual bool AddEventCallBack(const NET_EVENT_FUNCTOR_PTR &cb);

    virtual bool Update();

    virtual bool SendMsg(const std::string &msg, const socket_t sock, const bool text = true);
    virtual bool SendMsgToAllClient(const std::string &msg, const bool text = true);
    virtual bool SendMsgWithOutHead(const int16_t msg_id, const char *msg, const size_t len, const socket_t sock /*= 0*/);
    virtual bool SendMsgPB(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock);
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
};