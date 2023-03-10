#pragma once

#include <iostream>
#include <iosfwd>
#include <squick/core/queue.h>
#include <squick/core/consistent_hash.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/core/i_plugin_manager.h>

#include "i_net_client_module.h"
#include "i_net_module.h"

class NetClientModule : public INetClientModule
{
public:
    NetClientModule(IPluginManager* p);

    virtual bool Start();

    virtual bool AfterStart();

    virtual bool BeforeDestory();

    virtual bool Destory();

    virtual bool Update();

    virtual void AddServer(const ConnectData& xInfo);

    virtual unsigned int ExpandBufferSize(const unsigned int size = 1024 * 1024 * 20) override;

    virtual int AddReceiveCallBack(const SQUICK_SERVER_TYPES eType, NET_RECEIVE_FUNCTOR_PTR functorPtr);

    virtual int AddReceiveCallBack(const SQUICK_SERVER_TYPES eType, const uint16_t msgID, NET_RECEIVE_FUNCTOR_PTR functorPtr);

    virtual int AddEventCallBack(const SQUICK_SERVER_TYPES eType, NET_EVENT_FUNCTOR_PTR functorPtr);

    virtual void RemoveReceiveCallBack(const SQUICK_SERVER_TYPES eType, const uint16_t msgID);

    ////////////////////////////////////////////////////////////////////////////////
	virtual void SendByServerIDWithOutHead(const int serverID, const uint16_t msgID, const std::string& strData);

	virtual void SendByServerID(const int serverID, const uint16_t msgID, const std::string& strData);
	virtual void SendByServerID(const int serverID, const uint16_t msgID, const std::string& strData, const Guid id);

	virtual void SendToAllServerWithOutHead(const uint16_t msgID, const std::string& strData);

	virtual void SendToAllServer(const uint16_t msgID, const std::string& strData);
	virtual void SendToAllServer(const uint16_t msgID, const std::string& strData, const Guid id);

	virtual void SendToAllServerWithOutHead(const SQUICK_SERVER_TYPES eType, const uint16_t msgID, const std::string& strData);

	virtual void SendToAllServer(const SQUICK_SERVER_TYPES eType, const uint16_t msgID, const std::string& strData);
	virtual void SendToAllServer(const SQUICK_SERVER_TYPES eType, const uint16_t msgID, const std::string& strData, const Guid id);

	virtual void SendToServerByPB(const int serverID, const uint16_t msgID, const google::protobuf::Message& xData);
	virtual void SendToServerByPB(const int serverID, const uint16_t msgID, const google::protobuf::Message& xData, const Guid id);

    virtual void SendToAllServerByPB(const uint16_t msgID, const google::protobuf::Message& xData, const Guid id);

    virtual void SendToAllServerByPB(const SQUICK_SERVER_TYPES eType, const uint16_t msgID, const google::protobuf::Message& xData, const Guid id);

    ////////////////////////////////////////////////////////////////////////////////

	virtual void SendBySuitWithOutHead(const SQUICK_SERVER_TYPES eType, const std::string& strHashKey, const uint16_t msgID, const std::string& strData);

	virtual void SendBySuit(const SQUICK_SERVER_TYPES eType, const std::string& strHashKey, const uint16_t msgID, const std::string& strData);
	virtual void SendBySuit(const SQUICK_SERVER_TYPES eType, const std::string& strHashKey, const uint16_t msgID, const std::string& strData, const Guid id);

	virtual void SendBySuitWithOutHead(const SQUICK_SERVER_TYPES eType, const int nHashKey32, const uint16_t msgID, const std::string& strData);

	virtual void SendBySuit(const SQUICK_SERVER_TYPES eType, const int nHashKey32, const uint16_t msgID, const std::string& strData);
	virtual void SendBySuit(const SQUICK_SERVER_TYPES eType, const int nHashKey32, const uint16_t msgID, const std::string& strData, const Guid id);

	virtual void SendSuitByPB(const SQUICK_SERVER_TYPES eType, const std::string& strHashKey, const uint16_t msgID, const google::protobuf::Message& xData);
	virtual void SendSuitByPB(const SQUICK_SERVER_TYPES eType, const std::string& strHashKey, const uint16_t msgID, const google::protobuf::Message& xData, const Guid id);

	virtual void SendSuitByPB(const SQUICK_SERVER_TYPES eType, const int nHashKey32, const uint16_t msgID, const google::protobuf::Message& xData);
	virtual void SendSuitByPB(const SQUICK_SERVER_TYPES eType, const int nHashKey32, const uint16_t msgID, const google::protobuf::Message& xData, const Guid id);

    ////////////////////////////////////////////////////////////////////////////////

    virtual MapEx<int, ConnectData>& GetServerList();

    virtual SQUICK_SHARE_PTR<ConnectData> GetServerNetInfo(const SQUICK_SERVER_TYPES eType);

    virtual SQUICK_SHARE_PTR<ConnectData> GetServerNetInfo(const int serverID);

    virtual SQUICK_SHARE_PTR<ConnectData> GetServerNetInfo(const INet* pNet);

protected:

    void StartCallBacks(SQUICK_SHARE_PTR<ConnectData> pServerData);

    void ProcessUpdate();

private:
    void LogServerInfo();

    void KeepState(SQUICK_SHARE_PTR<ConnectData> pServerData);

    void OnSocketEvent(const SQUICK_SOCKET fd, const SQUICK_NET_EVENT eEvent, INet* pNet);

    int OnConnected(const SQUICK_SOCKET fd, INet* pNet);

    int OnDisConnected(const SQUICK_SOCKET fd, INet* pNet);

    void ProcessAddNetConnect();

private:
	int64_t mnLastActionTime;
	unsigned int mnBufferSize;
    //server_id, server_data
    NFConsistentHashMapEx<int, ConnectData> mxServerMap;
    //server_type, server_id, server_data
    MapEx<int, NFConsistentHashMapEx<int, ConnectData>> mxServerTypeMap;

    std::list<ConnectData> mxTempNetList;

    struct CallBack
    {
        //call back
		//std::map<int, NET_RECEIVE_FUNCTOR_PTR> mxReceiveCallBack;
		std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>> mxReceiveCallBack;
        std::list<NET_EVENT_FUNCTOR_PTR> mxEventCallBack;
        std::list<NET_RECEIVE_FUNCTOR_PTR> mxCallBackList;
    };

    MapEx<int, CallBack> mxCallBack;

	ILogModule* m_pLogModule;
};

