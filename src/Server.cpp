/**
 * @file Server.cpp
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 原来Connection中的响应函数的定义上升到Server中，实现服务器业务的定义
 * @version 0.1
 * @date 2024-12-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "Server.h"
#include "Channel.h"
#include "Connection.h"
#include "Acceptor.h"
#include "ThreadPool.h"
#include "EventLoop.h"
#include <functional>
#include <memory>
#include <string.h>
#include <unistd.h>

#define READ_BUFFER 1024

Server::Server(const char *ip, const int port) : nextConnid(1)
{
    mainReactor = std::make_unique<EventLoop>();
    acceptor = std::make_unique<Acceptor>(mainReactor.get(), ip, port);
    std::function<void(int)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);

    int size = 10; // std::thread::hardware_concurrency();
    thPool = std::make_unique<ThreadPool>(size);
    for (int i = 0; i < size; ++i) {
        std::unique_ptr<EventLoop> subReactor = std::make_unique<EventLoop>();
        subReactors.push_back(std::move(subReactor));
    }
}

Server::~Server() {}

void Server::start()
{
    for (int i = 0; i < subReactors.size(); ++i) {
        std::function<void()> cb = std::bind(&EventLoop::loop, subReactors[i].get());
        thPool->add(std::move(cb));
        // thPool->add(std::bind(&EventLoop::loop, subReactors[i])); // 移动语义，清晰、高效
    }
    mainReactor->loop();
}

void Server::newConnection(int clnt_fd)
{
    if (clnt_fd != -1) {
        int index = clnt_fd % subReactors.size();
        Connection *conn = new Connection(subReactors[index].get(), nextConnid, clnt_fd);
        std::function<void(int)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
        conn->setDeleteConnectionCallback(cb);
        conn->setOnConnectionCallback(connCallback);
        connections[clnt_fd] = conn;
        nextConnid++;
        nextConnid %= 1000;

        printf("New connection id: %d\n", nextConnid);
    }
}

void Server::deleteConnection(int fd){
    if(fd != -1){
        auto it = connections.find(fd);
        if(it != connections.end()){
            Connection *conn = connections[fd];
            connections.erase(fd);
            // close(sockfd);       //正常
            delete conn;     //会Segmant fault
            ::close(fd);
            conn = nullptr;
        }
    }
}

void Server::setOnConnectCallback(std::function<void(Connection *)> const &cb)
{
    connCallback = std::move(cb);
}

void Server::setOnNewConnectionCallback(std::function<void(Connection *)> const &cb)
{
    connCallback = std::move(cb);
}