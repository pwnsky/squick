#pragma once


#include <functional>
#include <list>
#include <vector>
#include "platform.h"

class IPlugin;
class IModule;
class IPluginManager;

typedef std::function<bool (IPluginManager* p, const std::string& fileName, std::string& content)> GET_FILECONTENT_FUNCTOR;

template<typename DerivedType, typename BaseType>
class TIsDerived
{
public:
    static int AnyFunction(BaseType* base)
    {
        return 1;
    }

    static  char AnyFunction(void* t2)
    {
        return 0;
    }

    enum
    {
        Result = (sizeof(int) == sizeof(AnyFunction((DerivedType*)NULL))),
    };
};

class ReplaceContent
{
public:
	ReplaceContent(const std::string content, const std::string newValue)
	{
		this->content = content;
		this->newValue = newValue;
	}
	std::string content;
	std::string newValue;
};

#define FIND_MODULE(classBaseName, className)  \
	assert((TIsDerived<classBaseName, IModule>::Result));

class IPluginManager
{
public:
    IPluginManager()
    {

    }


	virtual bool LoadPluginConfig()
	{
		return true;
	}

	virtual bool LoadPlugin()
	{
		return true;
	}

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

	/////////////////////

	template <typename T>
	T* FindModule()
	{
		IModule* pLogicModule = FindModule(typeid(T).name());
		if (pLogicModule)
		{
			if (!TIsDerived<T, IModule>::Result)
			{
				return NULL;
			}
            //TODO OSX上dynamic_cast返回了NULL
#if SQUICK_PLATFORM == SQUICK_PLATFORM_APPLE
            T* pT = (T*)pLogicModule;
#else
			T* pT = dynamic_cast<T*>(pLogicModule);
#endif
			assert(NULL != pT);

			return pT;
		}
		assert(NULL);
		return NULL;
	}

	template <typename T>
	void ReplaceModule(IModule* pModule)
	{
		IModule* pLogicModule = FindModule(typeid(T).name());
		if (pLogicModule)
		{
			RemoveModule(typeid(T).name());
		}


		AddModule(typeid(T).name(), pModule);
	};

	virtual bool ReLoadPlugin(const std::string& pluginDLLName) = 0;

    virtual void Registered(IPlugin* plugin) = 0;

    virtual void UnRegistered(IPlugin* plugin) = 0;

    virtual IPlugin* FindPlugin(const std::string& pluginName) = 0;

	virtual void AddModule(const std::string& moduleName, IModule* pModule) = 0;

	virtual void AddTestModule(const std::string& moduleName, IModule* pModule) = 0;

    virtual void RemoveModule(const std::string& moduleName) = 0;

    virtual IModule* FindModule(const std::string& moduleName) = 0;

    virtual IModule* FindTestModule(const std::string& moduleName) = 0;

	virtual std::list<IModule*> Modules() = 0;
	virtual std::list<IModule*> TestModules() = 0;

    virtual int GetAppID() const = 0;
    virtual void SetAppID(const int appID) = 0;

	virtual int GetAppType() const = 0;
	virtual void SetAppType(const int type) = 0;

    virtual bool IsRunningDocker() const = 0;
    virtual void SetRunningDocker(bool bDocker) = 0;



    virtual INT64 GetStartTime() const = 0;
    virtual INT64 GetNowTime() const = 0;

	virtual const std::string& GetConfigPath() const = 0;
	virtual void SetConfigPath(const std::string & strPath) = 0;

	virtual void SetConfigName(const std::string& fileName) = 0;
	virtual const std::string& GetConfigName() const = 0;

	virtual const std::string& GetAppName() const = 0;
	virtual void SetAppName(const std::string& appName) = 0;

	virtual const std::string& GetLogConfigName() const = 0;
	virtual void SetLogConfigName(const std::string& name) = 0;

	virtual IPlugin* GetCurrentPlugin() = 0;
	virtual IModule* GetCurrentModule() = 0;

	virtual void SetCurrentPlugin(IPlugin* pPlugin) = 0;
	virtual void SetCurrentModule(IModule* pModule) = 0;

	virtual int GetAppCPUCount() const = 0;
	virtual void SetAppCPUCount(const int count) = 0;

	virtual bool UsingBackThread() const = 0;
	virtual void SetUsingBackThread(const bool b) = 0;

	virtual void SetGetFileContentFunctor(GET_FILECONTENT_FUNCTOR fun) = 0;
	virtual bool GetFileContent(const std::string &fileName, std::string &content) = 0;

	virtual void AddFileReplaceContent(const std::string& fileName, const std::string& content, const std::string& newValue) = 0;
	virtual std::vector<ReplaceContent> GetFileReplaceContents(const std::string& fileName) = 0;


};

