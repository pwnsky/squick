
#pragma once

#include "platform.h"
#include "plugin_manager.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <time.h>
#include <utility>

#if PLATFORM != PLATFORM_WIN
#include <arpa/inet.h>
#include <execinfo.h>
#include <netdb.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

#if PLATFORM == PLATFORM_LINUX
#include <sys/prctl.h>
#endif

#endif

class PluginServer {
  public:
    PluginServer(const std::string &strArgv);
    virtual ~PluginServer() {}
    void Start();
    void Update();
    void Final();
    void Reload(int type);

    void SetBasicWareLoader(std::function<void(IPluginManager *p)> fun);
    void SetMidWareLoader(std::function<void(IPluginManager *p)> fun);

  private:
    std::shared_ptr<IPluginManager> pm_;
    std::string strArgvList;
    std::function<void(IPluginManager *p)> externalMidWarePluginLoader;
    std::function<void(IPluginManager *p)> externalBasicWarePluginLoader;

  private:
    void ProcessParameter();

    void StartDaemon();
    static bool GetFileContent(IPluginManager *p, const std::string &strFilePath, std::string &content);
};
