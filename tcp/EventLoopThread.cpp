/**
 * @file EventLoopThread.cpp
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 主线程-子线程的中转类，定义了从Reactor线程的创建以及启动
 * @version 0.1
 * @date 2025-01-03
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "EventLoopThread.h"
#include "EventLoop.h"
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>

EventLoopThread::EventLoopThread(): loop_(nullptr) {} // 主线程调用，无实际意义

EventLoopThread::~EventLoopThread() {}

/// @brief 子线程功能启动，主线程调用，创建子线程，启动子线程，等待子线程创建完毕后返回
/// @return 启动后的EventLoop
EventLoop* EventLoopThread::StartLoop(){
    thread_ = std::thread(std::bind(&EventLoopThread::ThreadFunc, this));
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr)
        {
            cond_.wait(lock); // = lock.unlock() + cond.wait() + lock.lock()
        }
        loop = loop_;
    }
    return loop;
}

/// @brief 子线程功能，创建EventLoop，启动EventLoop
void EventLoopThread::ThreadFunc(){
    // 这个loop是栈上的变量，出了作用域就会被销毁
    // 这个作用域正好对应了loop（子线程）的生命周期
    EventLoop loop;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop_->Loop();
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = nullptr;
    }
}