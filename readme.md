# CppWebServer Learning

跟随github项目，学习C++实现Web服务器。

## 原项目地址
[30daysCppWebServer] https://github.com/Wlgls/30daysCppWebServer

## day01 - socket实现
在Linux服务器上进行实现，gcc/g++ 7.5.0, cmake 3.10.2, VSCode 1.85.2

VSCode Git设置：
~~~bash
  git config --global user.email "you@example.com"
  git config --global user.name "Your Name"
~~~

socket使用: 
~~~cpp
#include <sys/socket.h>
// domain: 协议族，AF_INET (IPv4), AF_INET6 (IPv6), AF_UNIX
// type: 套接字类型，SOCK_STREAM (TCP流), SOCK_DGRAM (UDP报), SOCK_RAW
// protocol: 协议，通常为0，即自动设置
// 返回值：成功返回套接字描述符，失败返回-1
int sockfd = socket(int domain, int type, int protocol);

#include <arpa/inet.h>
struct sockaddr_in {
    sa_family_t sin_family; // 地址族，AF_INET
    uint16_t sin_port; // 端口号，网络字节序
    struct in_addr sin_addr; // IP地址，网络字节序
    char sin_zero[8]; // 填充，必须为0
};
// 绑定地址和socket （服务端）
bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
// 链接socket （客户端）
connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
// 监听socket, backlog: 最大连接数
listen(sockfd, backlog);
// 接受连接
int connfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
~~~

最简单的socket：客户端保存服务器的socket，服务端保存连接的socket，而后进行通信。