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
// 绑定地址和socket （服务端） 对两种socket类型都适用
bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
// 链接socket （客户端）
connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
// 监听socket, backlog: 最大连接数 （服务端） 同样对SOCK_STREAM类型适用
listen(sockfd, backlog);
// 接受连接 （服务端） 对SOCK_STREAM类型适用
int connfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
~~~

最简单的socket：客户端保存服务器的socket，服务端保存连接的socket，而后进行通信。

## day02 - error handling // message transfer

socket使用中，最通用的错误信号为调用函数返回-1，可以通过结合stdio的perror函数，对应各种错误位置进行错误输出。

~~~cpp
#include <sys/socket.h>
#include <stdio.h>

void errif(bool condition, const char *errmsg) {
    if (condition) {
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}

int sockfd = socket(AF_INET, SOCK_STREAM, 0);
errif(sockfd == -1, "socket create error");
errif(bind(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1, "socket bind error");
errif(listen(sockfd, SOMAXCONN) == -1, "socket listen error");
int clnt_sockfd = accept(sockfd, (sockaddr*)&clnt_addr, &clnt_addr_len);
errif(clnt_sockfd == -1, "socket accept error");
errif(connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1, "socket connect error");
~~~

socket的消息传送以文件接口为主, unistd.h 中的 read/write 函数用于TCP协议，sendto/recvfrom 函数用于UDP协议。

~~~cpp
#include <unistd.h>
char buffer[1024];
// read时返回的是读取的字节数，失败返回-1，返回0表示对端关闭连接
ssize_t read_bytes = read(int fd, void *buf, size_t count);
ssize_t write_bytes = write(int fd, const void *buf, size_t count);
~~~
