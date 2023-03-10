#pragma once

#include <squick/core/i_module.h>

class IAccountRedisModule
    : public IModule
{

public:
	virtual bool VerifyAccount(const std::string& account, const std::string& strPwd) = 0;
	virtual bool AddAccount(const std::string& account, const std::string& strPwd) = 0;
	virtual bool ExistAccount(const std::string& account) = 0;
};
