#include "squick.h"
#include <iostream>
#include <squick/core/base.h>
#include <squick/struct/struct.h>
#include <squick/core/plugin_server.h>
int main(int argc, char *argv[]) {
    // std::cout << "__cplusplus:" << __cplusplus << std::endl;

    // 强制加载squick_struct依赖
    void *libLoad = (void *)&SquickStruct::_Ident_default_instance_;

    // std::cout << libLoad << std::endl;

    std::vector<SQUICK_SHARE_PTR<PluginServer>> serverList; // 服务器列表

    std::string strArgvList;
    for (int i = 0; i < argc; i++) {
        strArgvList += " ";
        strArgvList += argv[i];
    }

    if (argc == 1) // 如果没加参数运行
    {
        /*
        std::cout << "<<  Squick  Help >>\n"
                "Usage: plugin=plugin_file server=server_name id=server_id [-d]\n"
                "Introduce:\n"
                "       plugin: Set your plugin file to load\n"
                "       server: Your server name, logger module will use it to log\n"
                "       id    : Set your server id, using it to load server configure informations\n"
                "       -d    : Run server in background\n"
                "Examples: ./squick plugin=test.xml server=defualt id=1\n";
        "\n";*/

        serverList.push_back(SQUICK_SHARE_PTR<PluginServer>(SQUICK_NEW PluginServer(strArgvList + " server=master id=1 plugin=master.xml")));
        serverList.push_back(SQUICK_SHARE_PTR<PluginServer>(SQUICK_NEW PluginServer(strArgvList + " server=login id=2 plugin=login.xml")));
        serverList.push_back(SQUICK_SHARE_PTR<PluginServer>(SQUICK_NEW PluginServer(strArgvList + " server=world id=3 plugin=world.xml")));
        serverList.push_back(SQUICK_SHARE_PTR<PluginServer>(SQUICK_NEW PluginServer(strArgvList + " server=db_proxy id=4 plugin=db_proxy.xml")));
        serverList.push_back(SQUICK_SHARE_PTR<PluginServer>(SQUICK_NEW PluginServer(strArgvList + " server=gateway id=5 plugin=gateway.xml")));
        serverList.push_back(SQUICK_SHARE_PTR<PluginServer>(SQUICK_NEW PluginServer(strArgvList + " server=game id=10 plugin=game.xml")));
        serverList.push_back(SQUICK_SHARE_PTR<PluginServer>(SQUICK_NEW PluginServer(strArgvList + " server=game id=11 plugin=game.xml")));
        serverList.push_back(
            SQUICK_SHARE_PTR<PluginServer>(SQUICK_NEW PluginServer(strArgvList + " server=gameplay_manager id=20 plugin=gameplay_manager.xml")));
        serverList.push_back(SQUICK_SHARE_PTR<PluginServer>(SQUICK_NEW PluginServer(strArgvList + " server=proxy id=6 plugin=proxy.xml")));

    } else {
        serverList.push_back(SQUICK_SHARE_PTR<PluginServer>(SQUICK_NEW PluginServer(strArgvList)));
    }

    for (auto item : serverList) {
        item->SetBasicWareLoader(BasicPluginLoader);
        item->SetMidWareLoader(MidWareLoader);
        item->Start();
    }

    ////////////////
    uint64_t nIndex = 0;
    while (true) {
        nIndex++;

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        for (auto item : serverList) {
            item->Update();
        }
    }

    ////////////////
    for (auto item : serverList) {
        item->Final();
    }

    serverList.clear();

    return 0;
}