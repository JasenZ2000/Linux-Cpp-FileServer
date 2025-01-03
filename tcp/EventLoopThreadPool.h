/**
 * @file EventLoopThreadPool.h
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 包装了原Server中的从线程初始化环节，并且记录运行信息。现在是ELT池，不再是传统线程池了
 * @version 0.1
 * @date 2025-01-03
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once
#include <functional>
#include <vector>
#include <memory>

#include <future>
#include "common.h"

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool
{
public:
    DISALLOW_COPY_AND_MOVE(EventLoopThreadPool);
    // 初始化功能
    EventLoopThreadPool(EventLoop*, int);
    ~EventLoopThreadPool();

    void SetThreadNum(int size);

    void Start();
    // 对Acceptor的接口
    EventLoop *GetNextLoop();

private:
    EventLoop *main_reactor_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    // EventLoop严格意义上受EventLoopThread管理，在这里随时可能变为nullptr
    std::vector<EventLoop*> loops_;
    
    int thread_nums_;
    int next_;
};