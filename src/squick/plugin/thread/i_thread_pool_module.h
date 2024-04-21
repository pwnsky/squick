#pragma once
#include <squick/core/i_module.h>
#include <thread>
#include <functional>

class ThreadTask;
typedef std::function<void(ThreadTask &)> TASK_PROCESS_FUNCTOR;

class ThreadTask {
  public:
    void Reset() {
        nTaskID = Guid();
        data = "";
        if (xThreadFunc) {
            xThreadFunc = TASK_PROCESS_FUNCTOR();
        }

        if (xEndFunc) {
            xEndFunc = TASK_PROCESS_FUNCTOR();
        }
    }

    Guid nTaskID;
    std::string data;
    TASK_PROCESS_FUNCTOR xThreadFunc;
    TASK_PROCESS_FUNCTOR xEndFunc;
};

class IThreadPoolModule : public IModule {
  public:
    virtual int GetThreadCount() = 0;

    template <typename BaseType> void DoAsyncTask(const Guid taskID, const std::string &data, BaseType *pBase, void (BaseType::*handler_begin)(ThreadTask &)) {
        TASK_PROCESS_FUNCTOR functor_begin = std::bind(handler_begin, pBase, std::placeholders::_1);

        DoAsyncTask(taskID, data, functor_begin, nullptr);
    }

    template <typename BaseType>
    void DoAsyncTask(const Guid taskID, const std::string &data, BaseType *pBase, void (BaseType::*handler_begin)(ThreadTask &),
                     void (BaseType::*handler_end)(ThreadTask &)) {
        TASK_PROCESS_FUNCTOR functor_begin = std::bind(handler_begin, pBase, std::placeholders::_1);
        TASK_PROCESS_FUNCTOR functor_end = std::bind(handler_end, pBase, std::placeholders::_1);

        DoAsyncTask(taskID, data, functor_begin, functor_end);
    }

    void DoAsyncTask(const Guid taskID, const std::string &data, TASK_PROCESS_FUNCTOR asyncFunctor) {
        TASK_PROCESS_FUNCTOR functor_end;
        DoAsyncTask(taskID, data, asyncFunctor, functor_end);
    }

    virtual void DoAsyncTask(const Guid taskID, const std::string &data, TASK_PROCESS_FUNCTOR asyncFunctor, TASK_PROCESS_FUNCTOR functor_end) = 0;

    /////repush the result
    virtual void TaskResult(const ThreadTask &task) = 0;
};