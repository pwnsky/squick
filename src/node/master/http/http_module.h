#pragma once

#include <squick/core/platform.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/net/i_http_server_module.h>
#include <squick/struct/struct.h>

#include "i_http_module.h"
#include <node/master/node/i_node_module.h>

namespace master::http {
class HttpModule : public IHttpModule {
  public:
    HttpModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
    }

    virtual bool Start();
    virtual bool Destory();

    virtual bool AfterStart();
    virtual bool Update();

  protected:
    bool OnCommandQuery(std::shared_ptr<HttpRequest> req);

    WebStatus OnFilter(std::shared_ptr<HttpRequest> req);

  private:
    IKernelModule *m_kernel_;
    ::IHttpServerModule *m_http_server_;
    node::INodeModule *m_node_;
    IClassModule *m_class_;
    IElementModule *m_element_;
};

} // namespace master::http_server