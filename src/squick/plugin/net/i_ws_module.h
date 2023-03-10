#pragma once

#include <iostream>
#include <iosfwd>
#include <squick/core/guid.h>
#include <squick/core/i_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/core/i_plugin_manager.h>
#include "i_net.h"
#include <squick/core/queue.h>
#ifdef _MSC_VER
#pragma warning(disable: 4244 4267)
#endif


////////////////////////////////////////////////////////////////////////////

class IWSModule
    : public IModule
{
public:

	template<typename BaseType>
	bool AddReceiveCallBack(const int msgID, BaseType* pBase, void (BaseType::*handleReceiver)(const SQUICK_SOCKET, const int, const char*, const uint32_t))
	{
		NET_RECEIVE_FUNCTOR functor = std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		NET_RECEIVE_FUNCTOR_PTR functorPtr(new NET_RECEIVE_FUNCTOR(functor));

		return AddReceiveCallBack(msgID, functorPtr);
	}

	template<typename BaseType>
	bool AddReceiveCallBack(BaseType* pBase, void (BaseType::*handleReceiver)(const SQUICK_SOCKET, const int, const char*, const uint32_t))
	{
		NET_RECEIVE_FUNCTOR functor = std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		NET_RECEIVE_FUNCTOR_PTR functorPtr(new NET_RECEIVE_FUNCTOR(functor));

		return AddReceiveCallBack(functorPtr);
	}

	template<typename BaseType>
	bool AddEventCallBack(BaseType* pBase, void (BaseType::*handler)(const SQUICK_SOCKET, const SQUICK_NET_EVENT, INet*))
	{
		NET_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		NET_EVENT_FUNCTOR_PTR functorPtr(new NET_EVENT_FUNCTOR(functor));

		return AddEventCallBack(functorPtr);
	}

	//as client
	virtual void Startialization(const char* ip, const unsigned short nPort) = 0;

	//as server
	virtual int Startialization(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount = 4) = 0;
	virtual unsigned int ExpandBufferSize(const unsigned int size = 1024 * 1024 * 20) = 0;

	virtual void RemoveReceiveCallBack(const int msgID) = 0;

	virtual bool AddReceiveCallBack(const int msgID, const NET_RECEIVE_FUNCTOR_PTR& cb) = 0;

	virtual bool AddReceiveCallBack(const NET_RECEIVE_FUNCTOR_PTR& cb) = 0;

	virtual bool AddEventCallBack(const NET_EVENT_FUNCTOR_PTR& cb) = 0;

	virtual bool Update() = 0;
    virtual bool SendMsgPB(const uint16_t msgID, const google::protobuf::Message& xData, const SQUICK_SOCKET sockIndex)=0;
	virtual bool SendMsg(const std::string& msg, const SQUICK_SOCKET sockIndex, const bool text = true) = 0;
	virtual bool SendMsgToAllClient(const std::string& msg, const bool text = true) = 0;

	virtual INet* GetNet() = 0;
};
