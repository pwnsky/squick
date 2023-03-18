

#ifndef SQUICK_THREAD_POOL_MODULE_H
#define SQUICK_THREAD_POOL_MODULE_H

#include "i_thread_pool_module.h"
#include <map>
#include <squick/core/platform.h>
#include <squick/core/queue.h>
#include <string>

class ThreadCell : MemoryCounter {
  public:
    ThreadCell(IThreadPoolModule *p) : MemoryCounter(GET_CLASS_NAME(ThreadCell), 1) {
        m_pThreadPoolModule = p;
        mThread = SQUICK_SHARE_PTR<std::thread>(SQUICK_NEW std::thread(&ThreadCell::Update, this));
    }

    void AddTask(const ThreadTask &task) { mTaskList.Push(task); }

    virtual void ToMemoryCounterString(std::string &info) override {}

  protected:
    void Update() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            {
                // pick the first task and do it
                ThreadTask task;
                while (mTaskList.TryPop(task)) {
                    if (task.xThreadFunc) {
                        task.xThreadFunc.operator()(task);
                    }

                    // repush the result to the main thread
                    // and, do we must to tell the result to the main thread?
                    if (task.xEndFunc) {
                        m_pThreadPoolModule->TaskResult(task);
                    }

                    task.Reset();
                }
            }
        }
    }

  private:
    Queue<ThreadTask> mTaskList;
    SQUICK_SHARE_PTR<std::thread> mThread;
    IThreadPoolModule *m_pThreadPoolModule;
};

class ThreadPoolModule : public IThreadPoolModule {
  public:
    ThreadPoolModule(IPluginManager *p);
    virtual ~ThreadPoolModule();

    virtual int GetThreadCount();

    virtual bool Start();

    virtual bool AfterStart();

    virtual bool BeforeDestory();

    virtual bool Destory();

    virtual bool Update();

    virtual void DoAsyncTask(const Guid taskID, const std::string &data, TASK_PROCESS_FUNCTOR asyncFunctor, TASK_PROCESS_FUNCTOR functor_end);

    virtual void TaskResult(const ThreadTask &task);

  protected:
    void UpdateTaskResult();

  private:
    Queue<ThreadTask> mTaskResult;
    std::vector<SQUICK_SHARE_PTR<ThreadCell>> mThreadPool;
};

#endif
