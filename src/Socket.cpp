#include <sys/socket.h>
#include <unistd.h>
#include "InetAddress.h"
#include "Socket.h"
#include "util.h"
#include <fcntl.h>
#include <string.h>

Socket::Socket() {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    errif(fd == -1, "socket create error");
}

Socket::Socket(int fd) : fd(fd) {
    errif(fd == -1, "socket create error");
}

Socket::~Socket() {
    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }    
}

void Socket::bind(InetAddress* addr) {
    struct sockaddr_in _addr = addr->getAddr();
    errif(::bind(fd, (sockaddr*)&_addr, sizeof(_addr)) == -1, "socket bind error");
}

void Socket::listen() {
    errif(::listen(fd, SOMAXCONN) == -1, "socket listen error");
}

void Socket::setnonblocking() {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int Socket::accept(InetAddress *_addr){
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr)); // 没有这个东西就会读不到正确的地址
    socklen_t addr_len = sizeof(addr);
    int clnt_sockfd = ::accept(fd, (sockaddr*)&addr, &addr_len);
    errif(clnt_sockfd == -1, "socket accept error");
    _addr->setInetAddr(addr);
    return clnt_sockfd;
}

int Socket::getFd() {
    return fd;
}

void Socket::connect(InetAddress* addr) {
    struct sockaddr_in _addr = addr->getAddr();
    errif(::connect(fd, (sockaddr*)&_addr, sizeof(_addr)) == -1, "socket connect error");
}