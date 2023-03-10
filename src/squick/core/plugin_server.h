
#ifndef SQUICK_SERVER_H
#define SQUICK_SERVER_H

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <utility>
#include <thread>
#include <chrono>
#include <future>
#include <functional>
#include <atomic>
#include "plugin_manager.h"
#include "platform.h"

#if SQUICK_PLATFORM != SQUICK_PLATFORM_WIN
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <execinfo.h>
#include <setjmp.h>

#if SQUICK_PLATFORM == SQUICK_PLATFORM_LINUX
#include <sys/prctl.h>
#endif

#endif


class PluginServer
{
public:
	PluginServer(const std::string& strArgv);

	virtual ~PluginServer()
	{
		Final();
	}

	void Start();
	void Update();
	void Final();

	void SetBasicWareLoader(std::function<void(IPluginManager * p)> fun);
	void SetMidWareLoader(std::function<void(IPluginManager * p)> fun);

private:
	SQUICK_SHARE_PTR<IPluginManager> pPluginManager;
	std::string strArgvList;
	std::function<void(IPluginManager * p)> externalMidWarePluginLoader;
	std::function<void(IPluginManager * p)> externalBasicWarePluginLoader;

private:

	void ProcessParameter();
	std::string FindParameterValue(const std::vector<std::string>& argList, const std::string& header);
	void StartDaemon();
	static bool GetFileContent(IPluginManager* p, const std::string& strFilePath, std::string& content);
};

#endif