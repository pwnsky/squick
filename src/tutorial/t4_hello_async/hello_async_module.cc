#include "hello_async_module.h"
#include <squick/core/queue.h>
#include <squick/struct/protocol_define.h>
#include <third_party/concurrentqueue/concurrentqueue.h>

bool HelloWorld4Module::Start() {
    m_pActorModule = pm_->FindModule<IActorModule>();
    m_thread_pool_ = pm_->FindModule<IThreadPoolModule>();

    return true;
}

void HelloWorld4Module::RequestAsyEnd(ActorMessage &actorMessage) {
    // std::cout << "Main thread: " << std::this_thread::get_id() << " Actor: " << actorMessage.id.ToString() << " MsgID: " << actorMessage.msg_id << " Data:" <<
    // actorMessage.data << std::endl;
}

bool HelloWorld4Module::AfterStart() {
    std::cout << "Hello, world4, AfterStart, Main thread: " << std::this_thread::get_id() << std::endl;

    ///////////////////////////
    std::cout << "start Benchmarks " << std::endl;
    // 100M
    int messageCount = 1000000;
    {
        std::cout << "Test for ConcurrentQueue" << std::endl;
        moodycamel::ConcurrentQueue<int> q;

        std::thread threads[6];

        // Producers
        for (int i = 0; i != 4; ++i) {
            threads[i] = std::thread([&]() {
                int64_t timeStart = SquickGetTimeMS();

                for (int j = 0; j != messageCount; ++j) {
                    q.enqueue(j);
                }

                int64_t timeEnd = SquickGetTimeMS();
                int64_t timeCost = timeEnd - timeStart;
                if (timeCost > 0) {
                    std::cout << "end Benchmarks, cost: " << timeCost << "ms for " << messageCount << ", qps: " << (messageCount / timeCost) * 1000
                              << std::endl;
                }
            });
        }

        // Consumers
        for (int i = 4; i != 6; ++i) {
            threads[i] = std::thread([&]() {
                int item;
                for (int j = 0; j != 20; ++j) {
                    // if (q.try_dequeue(item))
                    {}
                }
            });
        }

        // Wait for all threads
        for (int i = 0; i != 6; ++i) {
            threads[i].join();
        }

        // Collect any leftovers (could be some if e.g. consumers finish before producers)
        int item;
        // while (q.try_dequeue(item))
        {}
    }

    {
        std::cout << "Test for NFQuene" << std::endl;
        Queue<int> q;

        std::thread threads[6];
        int threadCount = 2;
        // Producers
        for (int i = 0; i != threadCount; ++i) {
            threads[i] = std::thread([&]() {
                int64_t timeStart = SquickGetTimeMS();

                for (int j = 0; j != messageCount; ++j) {
                    q.Push(j);
                }

                int64_t timeEnd = SquickGetTimeMS();
                int64_t timeCost = timeEnd - timeStart;
                if (timeCost > 0) {
                    std::cout << "end Benchmarks, cost: " << timeCost << "ms for " << messageCount << ", qps: " << (messageCount / timeCost) * 1000
                              << std::endl;
                }
            });
        }

        // Wait for all threads
        for (int i = 0; i != threadCount; ++i) {
            threads[i].join();
        }
    }
    {
        std::cout << "Test for NFQuene std::string" << std::endl;
        Queue<std::string> q;

        std::thread threads[6];
        int threadCount = 2;
        // Producers
        for (int i = 0; i != threadCount; ++i) {
            threads[i] = std::thread([&]() {
                int64_t timeStart = SquickGetTimeMS();

                for (int j = 0; j != messageCount; ++j) {
                    q.Push(std::to_string(j * j));
                }

                int64_t timeEnd = SquickGetTimeMS();
                int64_t timeCost = timeEnd - timeStart;
                if (timeCost > 0) {
                    std::cout << "end Benchmarks, cost: " << timeCost << "ms for " << messageCount << ", qps: " << (messageCount / timeCost) * 1000
                              << std::endl;
                }
            });
        }

        // Wait for all threads
        for (int i = 0; i != threadCount; ++i) {
            threads[i].join();
        }
    }
    {
        std::cout << "Test for Task NO RESULT!" << std::endl;
        int64_t timeStart = SquickGetTimeMS();
        // example 4
        for (int i = 0; i < messageCount; ++i) {
            m_thread_pool_->DoAsyncTask(Guid(), "sas", [&](ThreadTask &task) -> void {
                // std::cout << "example 4 thread id: " << std::this_thread::get_id() << " task id:" << task.nTaskID.ToString() << " task data:" << task.data <<
                // std::endl;
                task.data = "aaaaaresulttttttt";
            });
        }

        int64_t timeEnd = SquickGetTimeMS();
        int64_t timeCost = timeEnd - timeStart;
        if (timeCost > 0) {
            std::cout << "end Benchmarks, cost: " << timeCost << "ms for " << messageCount << ", qps: " << (messageCount / timeCost) * 1000 << std::endl;
        }
    }
    {
        std::cout << "Test for Task WITH RESULT!" << std::endl;
        int64_t timeStart = SquickGetTimeMS();
        // 100M
        // example 4
        for (int i = 0; i < messageCount; ++i) {
            m_thread_pool_->DoAsyncTask(
                Guid(), "sas",
                [&](ThreadTask &task) -> void {
                    // std::cout << "example 4 thread id: " << std::this_thread::get_id() << " task id:" << taskID.ToString() << " task data:" << strData <<
                    // std::endl;
                    task.data = "aaaaaresulttttttt";
                },
                [&](ThreadTask &task) -> void {
                    // std::cout << "example 4 thread id: " << std::this_thread::get_id() << " task id:" << taskID.ToString() << " task result:" << strData <<
                    // std::endl;
                });
        }

        int64_t timeEnd = SquickGetTimeMS();
        int64_t timeCost = timeEnd - timeStart;
        if (timeCost > 0) {
            std::cout << "end Benchmarks, cost: " << timeCost << "ms for " << messageCount << ", qps: " << (messageCount / timeCost) * 1000 << std::endl;
        }
    }

    // actor test
    {
        std::cout << "Test for actor mode" << std::endl;
        int64_t timeStart = SquickGetTimeMS();

        auto actorID1 = m_pActorModule->RequireActor();
        m_pActorModule->AddComponent<HttpComponent>(actorID1->ID());

        for (int i = 0; i < 5; ++i) {
            m_pActorModule->AddEndFunc(i, this, &HelloWorld4Module::RequestAsyEnd);
        }

        for (int i = 5; i < 10; ++i) {
            m_pActorModule->AddEndFunc(i, [](ActorMessage &actorMessage) -> void {
                // std::cout << "example 2 AddEndFunc " << actorMessage.id.ToString() << " MSGID: " << actorMessage.msg_id << std::endl;
            });
        }

        for (int i = 0; i < messageCount; ++i) {
            m_pActorModule->SendMsgToActor(actorID1->ID(), Guid(), i, "test");
            // m_pActorModule->SendMsgToActor(actorID1, i, std::to_string(i*i));
        }

        int64_t timeEnd = SquickGetTimeMS();
        int64_t timeCost = timeEnd - timeStart;
        if (timeCost > 0) {
            std::cout << "end Benchmarks, cost: " << timeCost << "ms for " << messageCount << ", qps: " << (messageCount / timeCost) * 1000 << std::endl;
        }
    }

    std::cout << "Hello, world4, AfterStart end" << std::endl;
    return true;
}

bool HelloWorld4Module::Update() {

    // std::cout << "Hello, world4, Update" << std::endl;

    return true;
}

bool HelloWorld4Module::BeforeDestory() {

    std::cout << "Hello, world4, BeforeDestory" << std::endl;

    return true;
}

bool HelloWorld4Module::Destory() {

    std::cout << "Hello, world4, Destory" << std::endl;

    return true;
}
