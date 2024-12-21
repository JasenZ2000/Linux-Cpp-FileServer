#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include "util.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL); // 先获取原来的文件描述符的状态
    int new_option = old_option | O_NONBLOCK; // 修改为非阻塞
    fcntl(fd, F_SETFL, new_option); // 再将原来的文件描述符的状态修改为非阻塞
    return;
}

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8888);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //bind(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr));
    errif(bind(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) == -1, "socket bind error");

    //listen(sockfd, SOMAXCONN);
    errif(listen(sockfd, SOMAXCONN) == -1, "socket listen error");

    // epoll 设置
    int epfd = epoll_create1(0); // 常态
    errif(epfd == -1, "epoll create error");
    struct epoll_event event[MAX_EVENTS], ev;
    bzero(&event, sizeof(event));
    bzero(&ev, sizeof(ev));
    ev.data.fd = sockfd;
    ev.events = EPOLLIN; // 水平触发;
    setnonblocking(sockfd);
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev); // 新连接事件

    while (true) {
        int nfds = epoll_wait(epfd, event, MAX_EVENTS, -1); // 阻塞
        errif(nfds == -1, "epoll wait error");
        for (int i = 0; i < nfds; ++i) {
            if (event[i].data.fd == sockfd) { // epfd监视新连接事件与消息事件
                struct sockaddr_in clnt_addr;
                bzero(&clnt_addr, sizeof(clnt_addr));
                socklen_t clnt_addr_len = sizeof(clnt_addr);

                int clnt_sockfd = accept(sockfd, (sockaddr *)&clnt_addr, &clnt_addr_len);
                errif(clnt_sockfd == -1, "socket accept error");
                printf("client ip: %s, port: %d\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

                bzero(&ev, sizeof(ev));
                ev.data.fd = clnt_sockfd;
                ev.events = EPOLLIN | EPOLLET; // 这里边缘触发效率更高
                setnonblocking(clnt_sockfd);
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sockfd, &ev); // 消息事件
                
            } else if (event[i].events & EPOLLIN) {
                char buf[READ_BUFFER];
                while (true) {
                    bzero(&buf, sizeof(buf));
                    ssize_t ret = read(event[i].data.fd, buf, sizeof(buf));
                    if (ret > 0) {
                        printf("client fd %d message: %s\n", event[i].data.fd, buf);
                        write(event[i].data.fd, buf, strlen(buf));
                    } else if (ret == -1 && errno == EINTR) { // 客户端正常中断
                        printf("fd %d continue reading\n", event[i].data.fd);
                        continue;
                    } else if (ret == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
                        printf("fd %d message all read, errno: %d\n", event[i].data.fd, errno);
                        break;
                    } else if (ret == 0) {
                        printf("client fd %d close\n", event[i].data.fd);
                        close(event[i].data.fd);
                        break;
                    }
                }
            } else {
                printf("something else happened\n");
            }
        }
    }
    
    close(sockfd);

    return 0;
}