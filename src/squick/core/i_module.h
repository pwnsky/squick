#pragma once

#include <string>
#include "i_plugin_manager.h"
#include "map.h"
#include "list.h"
#include "data_list.h"
#include "smart_enum.h"

class IModule
{

public:
    IModule() : m_bIsUpdate(false), pPluginManager(NULL)
    {
    }

    virtual ~IModule() {}

    virtual bool Awake()
    {
        return true;
    }

    virtual bool Start()
    {

        return true;
    }

    virtual bool AfterStart()
    {
        return true;
    }

    virtual bool CheckConfig()
    {
        return true;
    }

    virtual bool ReadyUpdate()
    {
        return true;
    }

    virtual bool Update()
    {
        return true;
    }

    virtual bool BeforeDestory()
    {
        return true;
    }

    virtual bool Destory()
    {
        return true;
    }

    virtual bool Finalize()
    {
        return true;
    }

	virtual bool OnReloadPlugin()
	{
		return true;
	}

    virtual IPluginManager* GetPluginManager() const
    {
        return pPluginManager;
    }

    std::string name;
    bool m_bIsUpdate;
protected:
	IPluginManager* pPluginManager;
};
