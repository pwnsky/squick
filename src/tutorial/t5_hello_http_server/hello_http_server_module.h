

#ifndef SQUICK_HELLO_WORLD_H
#define SQUICK_HELLO_WORLD_H

#include <iostream>

#include <squick/core/data_list.h>
#include <squick/core/i_module.h>
#include <squick/core/object.h>
#include <squick/core/platform.h>
#include <squick/struct/define.pb.h>

#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/net/export.h>

class IHelloWorld5 : public IModule {};

class HelloWorld5 : public IHelloWorld5 {
  public:
    HelloWorld5(IPluginManager *p) {
        m_bIsUpdate = true;
        pPluginManager = p;
    }

    virtual bool Start();
    virtual bool AfterStart();

    virtual bool Update();

    virtual bool BeforeDestory();
    virtual bool Destory();

  protected:
    bool OnCommandQuery(SQUICK_SHARE_PTR<HttpRequest> req);

    WebStatus OnFilter(SQUICK_SHARE_PTR<HttpRequest> req);

    int OnHeartBeat(const Guid &self, const std::string &heartBeat, const float time, const int count);

    void OnGetCallBack(const Guid id, const int state_code, const std::string &strRespData);
    void OnPostCallBack(const Guid id, const int state_code, const std::string &strRespData, const std::string &strMemoData);

    void OnWebSocketTestProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

    void OnTCPEvent(const SQUICK_SOCKET fd, const SQUICK_NET_EVENT event, INet *pNet);

    void OnLoginProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

  private:
    IClassModule *m_pLogicClassModule;
    IHttpServerModule *m_pHttpNetModule;
    IHttpClientModule *m_pHttpClientModule;
    IWSModule *m_pWSModule;
    IElementModule *m_pElementModule;
    IScheduleModule *m_pScheduleModule;
    INetModule *m_pNetModule;
};

#endif
