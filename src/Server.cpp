#include "Server.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Connection.h"
#include "Acceptor.h"
#include <functional>
#include <string.h>
#include <unistd.h>

#define READ_BUFFER 1024

Server::Server(EventLoop *_loop)
    : loop(_loop), acceptor(new Acceptor(_loop))
{
    std::function<void(Socket *)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);
}

Server::~Server() {
    delete acceptor;
}

void Server::newConnection(Socket *clnt_sock)
{
    if (clnt_sock->getFd() != -1) {
        Connection *conn = new Connection(loop, clnt_sock);
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