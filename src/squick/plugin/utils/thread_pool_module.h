#pragma once

#include "i_thread_pool_module.h"
#include <map>
#include <squick/core/platform.h>
#include <squick/core/queue.h>
#include <string>
#include <squick/core/base.h>

class ThreadCell : MemoryCounter {
  public:
    ThreadCell(IThreadPoolModule *p) : MemoryCounter(GET_CLASS_NAME(ThreadCell), 1) {
        m_thread_pool_ = p;
        mThread = std::shared_ptr<std::thread>(new std::thread(&ThreadCell::Update, this));
    }

    void AddTask(const ThreadTask &task) { mTaskList.Push(task); }

    virtual void ToMemoryCounterString(std::string &info) override {}

  protected:
    void Update() {
        ThreadTask task;
        while (true) {
            // 任务每0.1秒再执行
            std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_POOL_SLEEP_TIME));
            {
                // pick the first task and do it
                while (mTaskList.TryPop(task)) {
                    if (task.xThreadFunc) {
                        task.xThreadFunc.operator()(task);
                    }

                    // repush the result to the main thread
                    // and, do we must to tell the result to the main thread?
                    if (task.xEndFunc) {
                        m_thread_pool_->TaskResult(task);
                    }

                    task.Reset();
                }
            }
        }
    }

  private:
    Queue<ThreadTask> mTaskList;
    std::shared_ptr<std::thread> mThread;
    IThreadPoolModule *m_thread_pool_;
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
    std::vector<std::shared_ptr<ThreadCell>> mThreadPool;
};