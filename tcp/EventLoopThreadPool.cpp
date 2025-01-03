/**
 * @file EventLoopThreadPool.cpp
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief ELT池的实现，怪怪的，因为实际上发挥作用的时候，还是通过主线程直接访问EventLoop来实现的
 * @version 0.1
 * @date 2025-01-03
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"
#include <memory>

EventLoopThreadPool::EventLoopThreadPool(EventLoop* main , int size) 
    : main_reactor_(main), thread_nums_(size), next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::SetThreadNum(int size){
    thread_nums_ = size;
}

void EventLoopThreadPool::Start(){
    for (int i = 0; i < thread_nums_; i++)
    {
        std::unique_ptr<EventLoopThread> ptr = std::make_unique<EventLoopThread>();
        threads_.push_back(std::move(ptr));
        loops_.push_back(threads_.back()->StartLoop());
    }
}

EventLoop* EventLoopThreadPool::GetNextLoop(){
    EventLoop *loop = main_reactor_;
    if(!loops_.empty()){
        loop = loops_[next_];
        next_ = (next_ + 1) % static_cast<int>(loops_.size());
    }
    return loop;
}