#include "ThreadPool.h"

ThreadPool::ThreadPool(int threadNum) : stop(false)
{
    for (int i = 0; i < threadNum; i++)
    {
        threads.emplace_back(std::thread([this](){
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> locker(mtx);
                    cv.wait(locker, [this](){ return stop || !tasks.empty(); });
                    if (stop && tasks.empty()) return;
                    task = tasks.front();
                    tasks.pop();
                }
                task();
            }
        }));
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> locker(mtx);
        stop = true;
    }
    cv.notify_all();
    for (auto& thread : threads) {
        if (thread.joinable())
            thread.join();
    }
}
