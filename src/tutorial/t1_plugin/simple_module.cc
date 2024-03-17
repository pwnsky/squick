
#include "simple_module.h"

namespace tutorial {
bool SimpleModule::Start() {
    std::cout << "SimpleModule Start" << std::endl;
    return true;
}

bool SimpleModule::AfterStart() {
    std::cout << "SimpleModule AfterStart" << std::endl;
    return true;
}

// Update
bool SimpleModule::Update() {
    // 
    auto now_time = SquickGetTimeS();
    if(now_time - last_update_time_ > 2) {
        last_update_time_ = now_time;
        std::cout << "SimpleModule Update, time: " << now_time << std::endl;
    }
    return true;
}

bool SimpleModule::BeforeDestory() {
    std::cout << "SimpleModule BeforeDestory" << std::endl;
    return true;
}

bool SimpleModule::Destory() {
    std::cout << "SimpleModule Destory" << std::endl;
    return true;
}

int SimpleModule::MyExportFunc() {
    std::cout << "MyExportFunc called by others" << std::endl;
    return true;
}

}