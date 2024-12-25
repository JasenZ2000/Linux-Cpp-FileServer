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

void Server::newConnection(Socket *serv_sock)
{
    InetAddress* clnt_addr = new InetAddress();
    Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr));
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
    clnt_sock->setnonblocking();
    Connection *conn = new Connection(loop, clnt_sock);
    std::function<void(Socket*)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
    conn->setDeleteConnectionCallback(cb);
    connections[clnt_sock->getFd()] = conn;
}

void Server::deleteConnection(Socket *clnt_sock){
    Connection *conn = connections[clnt_sock->getFd()];
    connections.erase(clnt_sock->getFd());
    delete conn;
}