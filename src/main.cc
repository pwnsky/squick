#include <iostream>
#include <fstream>
#include <sstream>
#include <core/base.h>
#include <core/plugin_server.h>
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
    std::string configFile = "../config/default_startup.conf";
    std::ifstream file(configFile);
    
    if (!file.is_open()) {
        SQUICK_PRINT("Warning: Cannot open config file: " + configFile + ", using default hardcoded configuration");
        return;
    }
    
    std::string line;
    int loadedCount = 0;
    while (std::getline(file, line)) {
        // Trim leading and trailing whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            continue; // Skip empty lines
        }
        size_t end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);
        
        // Skip comment lines (lines starting with #)
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Add plugin server with configuration from file
        AddPluginServer(serverList, line);
        loadedCount++;
    }
    
    file.close();
    SQUICK_PRINT("Loaded " + std::to_string(loadedCount) + " plugin server(s) from " + configFile);
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

    if (argc == 1) {
#ifdef SQUICK_DEV
        DefaultStartUp(strArgvList, serverList);
        // TutorialStartUp(strArgvList, serverList);
#else
        PrintLogo();
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
    signal(SIGTERM, SquickExit);
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
        int state = GetSquickReloadState();
        for (auto item : serverList) {
            item->Update();
            if (state > 0)
            {
                item->Reload(state);
            }
        }
        if (state > 0)
        {
            SetSquickReloadState(0);
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
