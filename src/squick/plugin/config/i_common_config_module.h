

#ifndef SQUICK_INTF_COMMONCONFIG_MODULE_H
#define SQUICK_INTF_COMMONCONFIG_MODULE_H

#include <iostream>
#include <squick/core/i_module.h>

class ICommonConfigModule
    : public IModule
{
public:
	virtual bool LoadConfig(const std::string& strFile) = 0;
    virtual bool ClearConfig() = 0;

	virtual const int GetFieldInt(const std::string& key1, const std::string& field) = 0;
    virtual const int GetFieldInt(const std::string& key1, const std::string& key2, const std::string& field) = 0;

	virtual const std::string& GetFieldString(const std::string& key1, const std::string& field) = 0;
    virtual const std::string& GetFieldString(const std::string& key1, const std::string& key2, const std::string& field) = 0;

	virtual std::vector<std::string> GetFieldList(const std::string& key1) = 0;
	virtual std::vector<std::string> GetSubKeyList(const std::string& key1) = 0;
};

#endif