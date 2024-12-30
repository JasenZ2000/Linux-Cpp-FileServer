/**
 * @file Acceptor.h
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 合并了Socket，InetAddress的功能，加入智能指针
 * @version 0.1
 * @date 2024-12-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#include "common.h"
#include <memory>
#include <functional>

class EventLoop;
class Channel;

class Acceptor {
private:
    EventLoop* loop;
    int listenFd;
    std::unique_ptr<Channel> acceptChannel;
    std::function<void(int)> newConnectionCallback;
public:
    DISALLOW_COPY_AND_MOVE(Acceptor);
    Acceptor(EventLoop* loop, const char* ip, const int port);
    ~Acceptor();

    void create();
    void bind(const char* ip, const int port);
    void listen();

    void acceptConnection();
    void setNewConnectionCallback(std::function<void(int)> const &callback);
};