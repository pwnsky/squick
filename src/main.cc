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

void PrintLogo() {
    std::cout << R"(
             ,        ,
            /(        )`
            \ \___   / |
            /- _  `-/  '
           (/\/ \ \   /\
           / /   | `    \
           O O   ) /    |
           `-^--'`<     '
          (_.)  _  )   /
           `.___/`    /
             `-----' /
<----.     __ / __   \
<----|====O)))==) \) /====
<----'    `--' `.__,' \
             |        |
              \       /       /\
         ______( (_  / \______/
       ,'  ,-----'   |
       `--{__________)
                 _      _
 ___  __ _ _   _(_) ___| | __
/ __|/ _` | | | | |/ __| |/ /
\__ \ (_| | |_| | | (__|   <
|___/\__, |\__,_|_|\___|_|\_\
        |_|
)";
    std::cout << "Version: " << SQUICK_VERSION << "\nGithub : https://github.com/pwnsky/squick\n\n";
}

// Just for debug or dev
void DefaultStartUp(std::string strArgvList, std::vector<std::shared_ptr<PluginServer>> &serverList) {
    serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=master id=1")));

    serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=world id=100")));
    //serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=world id=101"))); // 区服2
    serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=db_proxy id=300")));

    //serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=db_proxy id=301"))); // 区服2

    serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=login id=2")));

    // Lobby
    serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=lobby id=1000")));
    serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=lobby id=1001")));
    //serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=lobby id=1002")));

    // Game
    //serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=game id=3000")));
    //serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=game id=3001")));
    //serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=game_mgr id=2000")));

    serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=proxy id=500")));
    //serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=proxy id=501")));

    //serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=proxy id=502"))); // 区服2
}

void TutorialStartUp(std::string strArgvList, std::vector<std::shared_ptr<PluginServer>>& serverList) {
    serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList + " type=tutorial/http")));
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

    PrintLogo();

    if (argc == 1) // 如果没加参数运行
    {
        /*
        std::cout << "<<  Squick  Help >>\n"
                "Usage: plugin=plugin_file server=server_name id=server_id [-d]\n"
                "Introduce:\n"
                "       type: Set your plugin file to load and this is your server name, logger module will use it to log\n"
                "       id    : Set your server id, using it to load server configure informations\n"
                "       -d    : Run server in background\n"
                "Examples: ./squick type=test.xml server=defualt id=1\n";
        "\n";*/

        DefaultStartUp(strArgvList, serverList);
        //TutorialStartUp(strArgvList, serverList);
    } else {
        serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList)));
    }

    for (auto item : serverList) {
        item->SetBasicWareLoader(BasicPluginLoader);
        item->SetMidWareLoader(MidWareLoader);
        item->Start();
    }
    uint64_t nIndex = 0;
    while (true) {
        nIndex++;

        std::this_thread::sleep_for(std::chrono::milliseconds(MAIN_THREAD_SLEEP_TIME));
        for (auto item : serverList) {
            item->Update();
        }
    }

    for (auto item : serverList) {
        item->Final();
    }

    serverList.clear();

    return 0;
}