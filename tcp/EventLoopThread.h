/**
 * @file EventLoopThread.h
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 连接从reactor和EventLoop的子线程封装，很纯粹的初始化用类
 * @version 0.1
 * @date 2025-01-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "common.h"
#include <mutex>
#include <thread>
#include <condition_variable>

class EventLoop;

class EventLoopThread {
public:
    DISALLOW_COPY_AND_MOVE(EventLoopThread);
    EventLoopThread();
    ~EventLoopThread();
    EventLoop *StartLoop();

private:
    void ThreadFunc();
    // 这个EventLoop在Muduo中受mutex保护
    // 利用作用域管理loop的生命周期
    EventLoop *loop_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
};