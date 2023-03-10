

#ifndef SQUICK_INTF_HTTP_NET_MODULE_H
#define SQUICK_INTF_HTTP_NET_MODULE_H

#include <squick/core/platform.h>
#include <squick/core/i_module.h>
#include "i_http_server.h"

class IHttpServerModule
	: public IModule
{
public:
	virtual ~IHttpServerModule() {};

	// register msg callback
	template<typename BaseType>
	bool AddRequestHandler(const std::string& strPath, const HttpType eRequestType, BaseType* pBase, bool (BaseType::*handleReceiver)(SQUICK_SHARE_PTR<HttpRequest> req))
	{
		HTTP_RECEIVE_FUNCTOR functor = std::bind(handleReceiver, pBase, std::placeholders::_1);
		HTTP_RECEIVE_FUNCTOR_PTR functorPtr(new HTTP_RECEIVE_FUNCTOR(functor));
		return AddMsgCB(strPath, eRequestType, functorPtr);
	}

	template<typename BaseType>
	bool AddNetFilter(const std::string& strPath, BaseType* pBase, WebStatus(BaseType::*handleFilter)(SQUICK_SHARE_PTR<HttpRequest> req))
	{
		HTTP_FILTER_FUNCTOR functor = std::bind(handleFilter, pBase, std::placeholders::_1);
		HTTP_FILTER_FUNCTOR_PTR functorPtr(new HTTP_FILTER_FUNCTOR(functor));

		return AddFilterCB(strPath, functorPtr);
	}
public:
	virtual int StartServer(const unsigned short nPort) = 0;

	virtual bool ResponseMsg(SQUICK_SHARE_PTR<HttpRequest> req, const std::string& msg, WebStatus code = WebStatus::WEB_OK, const std::string& reason = "OK") = 0;

private:
	virtual bool AddMsgCB(const std::string& strPath, const HttpType eRequestType, const HTTP_RECEIVE_FUNCTOR_PTR& cb) = 0;
	virtual bool AddFilterCB(const std::string& strPath, const HTTP_FILTER_FUNCTOR_PTR& cb) = 0;

};
#endif
