#pragma once

#include <iostream>
#include <core/guid.h>
#include <core/i_module.h>

class IErrorBusModule : public IModule {

  public:
    enum ErrID {
        ERR_NONE,
    };

    struct ErrorData {
        ErrorData() { errID = ErrID::ERR_NONE; }

        Guid id;
        ErrID errID;
        std::string data;
    };

    virtual ErrorData GetLastError() = 0;

    virtual void AddError(const Guid object, const ErrID err, const std::string &stData) = 0;

    virtual void ClearAllError() = 0;
};