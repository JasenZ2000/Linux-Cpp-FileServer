#include "Acceptor.h"
#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"

Acceptor::Acceptor(EventLoop* _loop)
    : loop(_loop) {
    listenSock = new Socket();
    servAddr = new InetAddress("127.0.0.1", 8888);
    listenSock->bind(servAddr);
    listenSock->listen();
    // listenSock->setnonblocking(); // 主线程上的Acceptor为何不设置非阻塞？
    acceptChannel = new Channel(loop, listenSock->getFd());
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    acceptChannel->setReadCallback(cb);
    acceptChannel->setUseThreadPool(false);
    acceptChannel->enableReading();
}

Acceptor::~Acceptor() {
    delete listenSock;
    delete servAddr;
    delete acceptChannel;
}

void Acceptor::acceptConnection() {
    InetAddress* clnt_addr = new InetAddress();
    Socket *clnt_sock = new Socket(listenSock->accept(clnt_addr));
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->getAddr().sin_addr), ntohs(clnt_addr->getAddr().sin_port));
    clnt_sock->setnonblocking();
    newConnectionCallback(clnt_sock); // 回调函数，在Server中定义(Server.cpp)
    delete clnt_addr; // socket可不能在这里删了
}

void Acceptor::setNewConnectionCallback(std::function<void(Socket*)> cb) {
    newConnectionCallback = cb;
}

