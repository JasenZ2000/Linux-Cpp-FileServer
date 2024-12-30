/**
 * @file Channel.h
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 主从Reactor里不需要Channel再把任务丢到线程池里，直接在Channel里处理即可
 * @version 0.1
 * @date 2024-12-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once
#include <sys/epoll.h>
#include <functional>
#include "common.h"

class EventLoop;
class Channel {
private:
    EventLoop* loop;
    int fd;
    uint32_t events;
    uint32_t revents;
    bool inEpoll;
    // bool useThreadPool;
    std::function<void()> writeCallback;
    std::function<void()> readCallback;
public:
    DISALLOW_COPY_AND_MOVE(Channel);
    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent();
    void enableReading();
    void enableWriting();
    void useET();
    void disableWriting();

    int getFd();
    uint32_t getEvents();
    uint32_t getRevents();

    bool getInEpoll();
    void setInEpoll(bool _in = true);

    void setRevents(uint32_t);
    void setReadCallback(std::function<void()> const &callback);
    void setWriteCallback(std::function<void()> const &callback);
    // void setUseThreadPool(bool use = true);
};
