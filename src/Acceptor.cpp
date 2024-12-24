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
    listenSock->setnonblocking();
    acceptChannel = new Channel(loop, listenSock->getFd());
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    acceptChannel->setCallback(cb);
    acceptChannel->enableReading();
}

Acceptor::~Acceptor() {
    delete listenSock;
    delete servAddr;
    delete acceptChannel;
}

void Acceptor::acceptConnection() {
    newConnectionCallback(listenSock);
}

void Acceptor::setNewConnectionCallback(std::function<void(Socket*)> cb) {
    newConnectionCallback = cb;
}