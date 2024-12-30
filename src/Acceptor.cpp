#include "Acceptor.h"
#include "Channel.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <memory>
#include "EventLoop.h"
#include "assert.h"
#include "unistd.h"
#include "util.h"

Acceptor::Acceptor(EventLoop* _loop, const char* ip, const int port)
    : loop(_loop),
      listenFd(-1)
{
    create();
    bind(ip, port);
    listen();
    acceptChannel = std::make_unique<Channel>(loop, listenFd);
    acceptChannel->setReadCallback(std::bind(&Acceptor::acceptConnection, this));
    acceptChannel->enableReading();
    printf("Server listening on fd: %d, IP: %s Port: %d\n", listenFd, ip, port);
}

Acceptor::~Acceptor() {
    loop->removeChannel(acceptChannel.get());
    ::close(listenFd);
}

void Acceptor::create() {
    assert(listenFd == -1);
    listenFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    errif(listenFd == -1, "socket create error");
}

void Acceptor::bind(const char* ip, const int port) {
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    errif(::bind(listenFd, (sockaddr*)&addr, sizeof(addr)) == -1, "socket bind error");
}

void Acceptor::listen() {
    errif(::listen(listenFd, SOMAXCONN) == -1, "socket listen error");
    // acceptChannel->enableReading();
}

void Acceptor::acceptConnection() {
    printf("accepting...\n");
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    socklen_t len = sizeof(addr);
    int clnt_sockfd = ::accept4(listenFd, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    errif(clnt_sockfd == -1, "socket accept error");

    if (newConnectionCallback) {
        newConnectionCallback(clnt_sockfd);
    }
}

void Acceptor::setNewConnectionCallback(std::function<void(int)> const &cb) {
    newConnectionCallback = std::move(cb);
}

