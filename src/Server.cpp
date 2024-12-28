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
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Connection.h"
#include "Acceptor.h"
#include "ThreadPool.h"
#include "EventLoop.h"
#include <functional>
#include <string.h>
#include <unistd.h>

#define READ_BUFFER 1024

Server::Server(EventLoop *_loop)
    : mainReactor(_loop), acceptor(new Acceptor(_loop))
{
    std::function<void(Socket *)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);

    int size = 10; // std::thread::hardware_concurrency();
    thPool = new ThreadPool(size);
    for (int i = 0; i < size; ++i) {
        subReactors.push_back(new EventLoop());
    }

    for (int i = 0; i < size; ++i) {
        thPool->add(std::bind(&EventLoop::loop, subReactors[i]));
    }
}

Server::~Server() {
    delete acceptor;
    delete thPool;
}

void Server::newConnection(Socket *clnt_sock)
{
    if (clnt_sock->getFd() != -1) {
        int index = clnt_sock->getFd() % subReactors.size();
        Connection *conn = new Connection(subReactors[index], clnt_sock);
        std::function<void(Socket*)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
        conn->setDeleteConnectionCallback(cb);
        conn->setOnConnectionCallback(connCallback);
        connections[clnt_sock->getFd()] = conn;
    }
}

void Server::deleteConnection(Socket* sockfd){
    int fd = sockfd->getFd();
    if(fd != -1){
        auto it = connections.find(fd);
        if(it != connections.end()){
            Connection *conn = connections[fd];
            connections.erase(fd);
            // close(sockfd);       //正常
            delete conn;         //会Segmant fault
        }
    }
}

void Server::setOnConnectCallback(std::function<void(Connection *)> cb)
{
    connCallback = std::move(cb);
}