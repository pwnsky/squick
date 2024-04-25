#include <iostream>
#include <squick/core/base.h>
#include <squick/core/plugin_server.h>
#include <struct/struct.h>

#if PLATFORM == PLATFORM_WIN
#include <windows.h>
#else
#include <signal.h>
#endif

void BasicPluginLoader(IPluginManager *pm_) {
    // Load your basic plugins
}

void MidWareLoader(IPluginManager *pm_) {
    // Load your midware plugins
}

void PrintLogo() {
    SQUICK_PRINT(R"(
                            ..                             
                             .-=.                ..   .        
                           .++.             .-*****#*+*= :     
                         .++:     -.  .=++++**: :=   -#%#*:    
       :-       -   .- .***#%%%##%*.  :-.               +##    
       :#      +=.=:%%#*=*@@%##%%%#+=*+.                 .#*.  
        +%+.  +@*#@%%%+=#@@@%***=++*%@@%#+.               +#   
         =%%=-%%**%#+==*%#+====*##%%%%%%%##*-             **.  
          -%@@%#%#+==+**==**%@@@@@%##%##%##%%#=          -*=   
          -%%%%#%#%%%##%%@@@@@@@@@%*==*###%##*=.        .+*    
        --=#%%%@%==*****#%%@%###******++**#%%#%*:      .++.    
        @###********++*****#%@#**+*++***+*##%%%#.    :-++:     
       .@@#**+****##%@%**#@@@@%###***++*#####%@@+ .-===+       
        +%*++#%+#@%@@%***%%%*#%@@@@%**+**##*#%%@@*===-.:       
       :=+*@@@@@@@@#+***###%@@@@@@@@%#***#%##%%%#+:.   ..      
     ::. :=%@@*+=+*#*#%@@@%%#%%@@@@@@%#***##%%%@%=     :.      
        :+%%#=++****######*++-=#@@@@@#+****#%%%@@#.    =.      
        +%@%#+*#%@@@@%****+====*@@@@@#*****%%%@@##:   .#:      
      .:%@@@@%%@@@@%+ .  .:-...*@@@@@%#****#%@@@+:    +*       
      .+*%%@@@@@@@#-*=. ..   -%@@@@@%***##%@@@@@-  . =*        
     :***++@@@@@@@%=   -:  .+@@@@@@%#**##%%@@@#=  :-*%=        
     +*=.  .%@@@@@@#.:-=:.*@@@@@@@#***#%%@@@@@-  -#%%=         
    .*:     -%@@@@@%%##+*%@@@@@%%####%%@@@@@%#+:+%%%=          
    .+       .%@@@@@%@@@@@@@@#####*#%@@@@@@%#+*%%%%-           
     =         -#@@@@@%@%%%#####%%@@@@@#+#@@%%%%%#=.           
     :       .. .=#%@@@@@@@@@@@@@@@%%%@@%%%%%%#+.              
      .        .. .=%@@@@@%%@@%%%%##%%%%%%%%*-                 
                 -%@@%%%%##%%%%#%%%%#%%#*-.                    
              :=**#*+-.-#**%%%#%%%%*#+.                        
                      -#*#%%%%#*#%%%***.                       
                           =@%%%%%+.                           
                           .*%%%%*.                            
                             :++:                              
                                                               
                 _      _
 ___  __ _ _   _(_) ___| | __
/ __|/ _` | | | | |/ __| |/ /
\__ \ (_| | |_| | | (__|   <
|___/\__, |\__,_|_|\___|_|\_\
        |_|
)");
    std::cout << "Version: " << SQUICK_VERSION << "\nGithub : https://github.com/pwnsky/squick\n\n";
}

void AddPluginServer(std::vector<std::shared_ptr<PluginServer>> &serverList, const std::string arg) {
    serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(arg)));
}
// Just for debug or dev
void DefaultStartUp(std::string strArgvList, std::vector<std::shared_ptr<PluginServer>> &serverList) {
    AddPluginServer(serverList, "type=master id=1 area=0 ip=127.0.0.1 port=10001 http_port=8888");
    AddPluginServer(serverList, "type=web id=10 area=0 ip=127.0.0.1 port=10010 http_port=8888 master_ip=127.0.0.1 master_port=10001");
    AddPluginServer(serverList, "type=world id=100 area=0 ip=127.0.0.1 port=10101 master_ip=127.0.0.1 master_port=10001");
    AddPluginServer(serverList, "type=db_proxy id=300 area=0 ip=127.0.0.1 port=10201 master_ip=127.0.0.1 master_port=10001");
    AddPluginServer(serverList, "type=login id=2 area=0 ip=127.0.0.1 port=10301 http_port=8088 master_ip=127.0.0.1 master_port=10001");
    AddPluginServer(serverList, "type=player id=1000 area=0 ip=127.0.0.1 port=10401 master_ip=127.0.0.1 master_port=10001");
    AddPluginServer(serverList, "type=player id=1001 area=0 ip=127.0.0.1 port=10402 master_ip=127.0.0.1 master_port=10001");
    AddPluginServer(serverList, "type=proxy id=500 area=0 ip=127.0.0.1 port=10501 ws_port=10502 master_ip=127.0.0.1 master_port=10001");
    
}

void TutorialStartUp(std::string strArgvList, std::vector<std::shared_ptr<PluginServer>> &serverList) {
    AddPluginServer(serverList, "type=tutorial/t1_plugin");
    AddPluginServer(serverList, "type=tutorial/t5_http");
}

bool squick_loop_ = true;
void SquickExit(int sig) { squick_loop_ = false; }

#if PLATFORM == PLATFORM_WIN
BOOL WINAPI HandlerRoutine(DWORD ctrl_type) {
    if (ctrl_type == CTRL_C_EVENT || ctrl_type == CTRL_CLOSE_EVENT || ctrl_type == CTRL_LOGOFF_EVENT || ctrl_type == CTRL_SHUTDOWN_EVENT) {
        SquickExit(0);
    } else {
        std::cout << "The handler sig is not surpported: " << ctrl_type << std::endl;
    }
    return true;
}
#endif

int main(int argc, char *argv[]) {
    // Force load struct.so
    void *libLoad = (void *)&rpc::_Vector3_default_instance_;
    std::vector<std::shared_ptr<PluginServer>> serverList;
    std::string strArgvList;
    for (int i = 0; i < argc; i++) {
        strArgvList += " ";
        strArgvList += argv[i];
    }

    PrintLogo();

    if (argc == 1) {
#ifdef SQUICK_DEV
        DefaultStartUp(strArgvList, serverList);
        // TutorialStartUp(strArgvList, serverList);
#else
        SQUICK_PRINT("<<  Squick  Help >>\n"
                     "Squick args usage:\n"
                     "       type: Set your app type;                                    default: proxy\n"
                     "       -d    : Run squick in background                            default: not set\n"
                     "       id    : Set your node id;                                   default: id=1\n"
                     "       type  : node type to run                                    default: type=proxy\n"
                     "       area  : The current node area;                              default: area=0\n"
                     "       ip    : The current node network ip;                        default: ip=127.0.0.1\n"
                     "       port  : The current node network port;                      default: port=10002\n"
                     "       public_ip    : The current node network public ip;          default: public_ip=127.0.0.1\n"
                     "       http_port    : The current node http port;                  default: http_port=80\n"
                     "       https_port   : The current node https port;                 default: https_port=443\n"
                     "       master_ip    : The master network ip for node connection;   default: master_ip=127.0.0.1\n"
                     "       master_port  : The master network port for node connection; default: master_port=10001\n"
                     "       logshow      : Is open log output to stdout;                default: logshow=1\n"
                     "Examples: ./squick type=master id=1 area=0 ip=127.0.0.1 port=10001 http_port=8888\n");
        "\n";
#endif // DEBUG
    } else {
        serverList.push_back(std::shared_ptr<PluginServer>(new PluginServer(strArgvList)));
    }

#if PLATFORM == PLATFORM_WIN
    SetConsoleCtrlHandler(HandlerRoutine, TRUE);
#else
    signal(SIGINT, SquickExit);
#endif

    for (auto item : serverList) {
        item->SetBasicWareLoader(BasicPluginLoader);
        item->SetMidWareLoader(MidWareLoader);
        item->Start();
    }

    SQUICK_PRINT("Squick has started!");
    uint64_t nIndex = 0;
    while (squick_loop_) {
        nIndex++;
        SetSquickMainThreadSleep(true);
        for (auto item : serverList) {
            item->Update();
        }
        if (IsSquickMainThreadSleep()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(MAIN_THREAD_SLEEP_TIME));
        }
    }

    SQUICK_PRINT("Squick Exiting ...");
    for (auto item : serverList) {
        item->Final();
    }
    serverList.clear();
    SQUICK_PRINT("Squick Exited ....");

    return 0;
}
