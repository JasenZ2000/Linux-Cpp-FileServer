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

    int size = 20; // std::thread::hardware_concurrency();
    thPool = new ThreadPool(size);
    for (int i = 0; i < size; ++i) {
        EventLoop *loop = new EventLoop();
        subReactors.push_back(loop);
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
        std::function<void(int)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
        conn->setDeleteConnectionCallback(cb);
        connections[clnt_sock->getFd()] = conn;
    }
}

void Server::deleteConnection(int sockfd){
    if(sockfd != -1){
        auto it = connections.find(sockfd);
        if(it != connections.end()){
            Connection *conn = connections[sockfd];
            connections.erase(sockfd);
            // close(sockfd);       //正常
            delete conn;         //会Segmant fault
        }
    }
}