/**
 * @file Channel.h
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 
 * @version 0.1
 * @date 2024-12-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once
#include <sys/epoll.h>
#include <functional>

class EventLoop;
class Channel {
private:
    EventLoop* loop;
    int fd;
    uint32_t events;
    uint32_t revents;
    bool inEpoll;
    bool useThreadPool;
    std::function<void()> writeCallback;
    std::function<void()> readCallback;
public:
    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent();
    void enableReading();

    int getFd();
    uint32_t getEvents();
    uint32_t getRevents();

    bool getInEpoll();
    void setInEpoll(bool _in = true);
    void useET();

    void setRevents(uint32_t);
    void setReadCallback(std::function<void()>);
    void setUseThreadPool(bool use = true);
};
