

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
        is_update_ = true;
        pm_ = p;
    }

    virtual bool Start();
    virtual bool AfterStart();

    virtual bool Update();

    virtual bool BeforeDestory();
    virtual bool Destory();

  protected:
    bool OnCommandQuery(std::shared_ptr<HttpRequest> req);

    WebStatus OnFilter(std::shared_ptr<HttpRequest> req);

    int OnHeartBeat(const Guid &self, const std::string &heartBeat, const float time, const int count);

    void OnGetCallBack(const Guid id, const int state_code, const std::string &strRespData);
    void OnPostCallBack(const Guid id, const int state_code, const std::string &strRespData, const std::string &strMemoData);

    void OnWebSocketTestProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    void OnTCPEvent(const socket_t fd, const SQUICK_NET_EVENT event, INet *pNet);

    void OnLoginProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  private:
    IClassModule *m_class_;
    IHttpServerModule *m_http_server_;
    IHttpClientModule *m_pHttpClientModule;
    IWSModule *m_pWSModule;
    IElementModule *m_element_;
    IScheduleModule *m_schedule_;
    INetModule *m_net_;
};

#endif
