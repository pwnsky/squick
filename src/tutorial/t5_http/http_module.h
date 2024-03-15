
#pragma once

#include <iostream>
#include <squick/core/base.h>
#include <squick/plugin/config/export.h>
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
  private:
    IClassModule *m_class_;
    IHttpServerModule *m_http_server_;
    IHttpClientModule * m_http_client_;
    IElementModule *m_element_;
};
