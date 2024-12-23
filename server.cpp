#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include <memory>
#include "util.h"
#include "Epoll.h"
#include "Socket.h"
#include "InetAddress.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void handleReadEvent(int fd);

int main()
{
    // Socket* serv_sock = new Socket(); // 使用new的话delete是必须的
    // InetAddress* addr = new InetAddress("127.0.0.1", 8888);
    std::unique_ptr<Socket> serv_sock = std::make_unique<Socket>(); // 智能指针是最优方式
    std::unique_ptr<InetAddress> addr = std::make_unique<InetAddress>("127.0.0.1", 8888);

    serv_sock->bind(addr.get());
    serv_sock->listen();
    serv_sock->setnonblocking();

    Epoll *ep = new Epoll();
    ep->addFd(serv_sock->getFd(), EPOLLIN); // 监听新连接事件

    while (true)
    {
        std::vector<epoll_event> activeEvents = ep->poll();
        int nums = activeEvents.size();
        for (int i = 0; i < nums; ++i)
        {
            if (activeEvents[i].data.fd == serv_sock->getFd())
            { // 新连接事件
                InetAddress* clnt_addr = new InetAddress(); // 智能指针是最优方式, 但是这里不使用智能指针，尽管有内存泄漏，但是不会影响程序的运行
                Socket* clnt_sock = new Socket(serv_sock->accept(clnt_addr)); // 原因是连接的生命周期需要独立管理
                // std::unique_ptr<InetAddress> clnt_addr = std::make_unique<InetAddress>();
                // std::unique_ptr<Socket> clnt_sock = std::make_unique<Socket>(serv_sock->accept(clnt_addr.get()));
                printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getFd(),
                       inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
                clnt_sock->setnonblocking();
                ep->addFd(clnt_sock->getFd(), EPOLLIN | EPOLLET); // 消息事件
            }
            else if (activeEvents[i].events & EPOLLIN)
            { // 来自客户端socket的消息事件
                handleReadEvent(activeEvents[i].data.fd);
            }
            else
            {
                printf("something else happened\n");
            }
        }
    }
}

void handleReadEvent(int fd)
{
    char buf[READ_BUFFER];
    while (true)
    {
        bzero(&buf, sizeof(buf));
        ssize_t ret = read(fd, buf, sizeof(buf));
        if (ret > 0)
        {
            printf("client fd %d message: %s\n", fd, buf);
            write(fd, buf, strlen(buf));
        }
        else if (ret == -1 && errno == EINTR)
        { // 客户端正常中断
            printf("fd %d continue reading\n", fd);
            continue;
        }
        else if (ret == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        {
            printf("fd %d message all read, errno: %d\n", fd, errno);
            break;
        }
        else if (ret == 0)
        {
            printf("client fd %d close\n", fd);
            close(fd); // 关闭socket等同于将其从epoll中删除
            break;
        }
    }
}