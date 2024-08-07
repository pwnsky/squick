
#include "navigation_module.h"

bool HelloWorld6::AfterStart() {
    m_pNavigationModule = pm_->FindModule<INavigationModule>();

    int sceneId = 1;
    Vector3 start(0, 0, 0);
    Vector3 end(23, 0, 5);
    vector<Vector3> pathResult;

    int ret = m_pNavigationModule->FindPath(sceneId, start, end, pathResult);
    if (ret > 0) {
        std::cout << "FindPath Success" << std::endl;
    } else {
        std::cout << "FindPath Fail" << std::endl;
    }
    return true;
}
