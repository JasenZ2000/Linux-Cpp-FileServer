#include <sys/socket.h>
#include <unistd.h>
#include "InetAddress.h"
#include "Socket.h"
#include "util.h"
#include <fcntl.h>

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
    errif(::bind(fd, (sockaddr*)&addr->addr, addr->addr_len) == -1, "socket bind error");
}

void Socket::listen() {
    errif(::listen(fd, SOMAXCONN) == -1, "socket listen error");
}

void Socket::setnonblocking() {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int Socket::accept(InetAddress* addr) {
    int connfd = ::accept(fd, (sockaddr*)&addr->addr, &addr->addr_len);
    errif(connfd == -1, "socket accept error");
    return connfd;
}

int Socket::getFd() {
    return fd;
}