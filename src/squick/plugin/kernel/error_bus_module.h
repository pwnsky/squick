

#ifndef SQUICK_ERROR_BUS_MODULE_H
#define SQUICK_ERROR_BUS_MODULE_H

#include "i_error_bus_module.h"
#include <iostream>

class ErrorBusModule : public IErrorBusModule {
  public:
    ErrorBusModule(IPluginManager *p) { pm_ = p; }

    virtual ~ErrorBusModule() {}

    virtual ErrorData GetLastError();

    virtual void AddError(const Guid object, const ErrID err, const std::string &stData);

    virtual void ClearAllError();

  private:
  private:
    std::list<ErrorData> mErrList;
};

#endif