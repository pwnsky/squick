#pragma once
#include <squick/core/i_module.h>

class IPlayerRedisModule
    : public IModule
{

public:
	virtual bool ExistRoleName(const std::string& strRoleName) = 0;
	virtual bool CreateRole(const std::string& account, const std::string& strRoleName, const Guid& id, const int nHomeSceneID) = 0;
	virtual bool GetRoleInfo(const std::string& account, std::string& strRoleName, Guid& id) = 0;

};
