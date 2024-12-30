/**
 * @file EventLoop.h
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 主从Reactor内事件触发通过channel与线程池通信，可以分离
 * @version 0.1
 * @date 2024-12-30
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once
#include <functional>
#include <memory>
#include "common.h"

class Epoll;
class Channel;
class EventLoop {
private:
    std::unique_ptr<Epoll> ep;
    bool quit;
public:
    DISALLOW_COPY_AND_MOVE(EventLoop);
    EventLoop();
    ~EventLoop();

    void loop();
    void updateChannel(Channel *ch);
    void removeChannel(Channel *ch);

    // void addThread(std::function<void()>);
};