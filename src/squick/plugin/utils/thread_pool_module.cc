

#include "thread_pool_module.h"

ThreadPoolModule::ThreadPoolModule(IPluginManager *p) {
    is_update_ = true;
    pm_ = p;
}

ThreadPoolModule::~ThreadPoolModule() {
    for (auto& t : mThreadPool) {
        t->Quit();
    }
}

bool ThreadPoolModule::Start() {
    for (int i = 0; i < pm_->GetAppCPUCount(); ++i) {
        mThreadPool.push_back(std::shared_ptr<ThreadCell>(new ThreadCell(this)));
    }

    return true;
}

bool ThreadPoolModule::AfterStart() { return true; }

bool ThreadPoolModule::BeforeDestroy() {

    return true;
}

bool ThreadPoolModule::Destroy() { return true; }

bool ThreadPoolModule::Update() {
    UpdateTaskResult();

    return true;
}

void ThreadPoolModule::DoAsyncTask(const Guid taskID, const std::string &data, TASK_PROCESS_FUNCTOR asyncFunctor, TASK_PROCESS_FUNCTOR functor_end) {
    ThreadTask task;
    task.nTaskID = taskID;
    task.data = data;
    task.xThreadFunc = asyncFunctor;
    task.xEndFunc = functor_end;

    size_t index = 0;
    if (!taskID.IsNull()) {
        index = taskID.nData64 % mThreadPool.size();
    }

    std::shared_ptr<ThreadCell> threadobject = mThreadPool[index];
    threadobject->AddTask(task);
}

void ThreadPoolModule::UpdateTaskResult() {
    ThreadTask xMsg;
    while (mTaskResult.TryPop(xMsg)) {
        if (xMsg.xEndFunc) {
            xMsg.xEndFunc.operator()(xMsg);
        }
    }
}

void ThreadPoolModule::TaskResult(const ThreadTask &task) { mTaskResult.Push(task); }

int ThreadPoolModule::GetThreadCount() { return (int)mThreadPool.size(); }
