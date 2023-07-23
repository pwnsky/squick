#pragma once
#include <squick/core/base.h>
namespace login::mysql {
class IMysqlModule : virtual public IModule {
  public:
    enum class AccountType {
        Account,
        Email,
        Phone,
        Wechat,
        QQ,
    };
    virtual bool RegisterAccount(const std::string &guid, const std::string &account, const std::string &password) = 0;
    virtual bool IsHave(const std::string &column_name, const std::string &value) = 0;
    virtual Guid GetGuid(AccountType type, const std::string &account) = 0;
};

} // namespace login::mysql