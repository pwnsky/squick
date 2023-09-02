#include <iostream>
#include <squick/core/base.h>
#include <squick/core/plugin_server.h>
#include <struct/struct.h>

void BasicPluginLoader(IPluginManager *pm_) {
    // 可自行设定加载的插件
}

void MidWareLoader(IPluginManager *pm_) {
    // 可自行设定加载的中间件插件
}

int main(int argc, char *argv[]) {
    // std::cout << "__cplusplus:" << __cplusplus << std::endl;

    // 强制加载squick_struct依赖
    void *libLoad = (void *)&rpc::_Vector3_default_instance_;

    // std::cout << libLoad << std::endl;

    std::vector<std::shared_ptr<PluginServer>> serverList; // 服务器列表

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

        serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=tester test=proxy id=100000")));
    } else {
        serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList)));
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