#pragma once
#include <functional>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

class ThreadPool
{
private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop;
public:
    ThreadPool(int num_threads = 4);
    ~ThreadPool();

    template <class F, class... Args>
    auto add(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>;
};

template <class F, class... Args>
// F:可调用（函数）类型，Args:可调用的参数类型 返回一个future，其利用result_of获取可调用对象的返回类型。
auto ThreadPool::add(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;
    // 外包智能指针负责传递与调用中的内存管理，内包packaged_task负责后续用get_future()绑定到future
    // 使用bind绑定函数f与参数，生成一个无参的可调用对象，返回一个future
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(mtx);

        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");
        tasks.emplace([task]() { (*task)(); });
    }
    cv.notify_one();
    return res;
}