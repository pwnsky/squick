#pragma once

#include <squick/core/base.h>

namespace login::mysql {

class IMysqlModule : public IModule {
  public:
    virtual bool RegisterAccount(const std::string& guid, const std::string& account, const std::string &password) = 0;
    virtual bool IsHave(const std::string& column_name, const std::string& value) = 0;

};

} // namespace login::logic