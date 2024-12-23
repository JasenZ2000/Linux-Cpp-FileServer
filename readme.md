# CppWebServer Learning

跟随github项目，学习C++实现Web服务器。

## 原项目地址
[30daysCppWebServer] https://github.com/Wlgls/30daysCppWebServer

## To Learn Sth More

1、C++异常处理；2、UDP的消息传输；3、非阻塞式socket

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

## day03 - 简单高并发 // IO多路复用

IO复用是指通过单个线程，同时监听多个IO事件，当有IO事件发生时，通知程序进行处理。在Linux中，IO复用的实现有select、poll、epoll等。

~~~cpp
// select:将文件描述符集合fd_set拷贝到内核空间，内核遍历所有文件描述符，当有IO事件发生时，将文件描述符从内核空间拷贝回用户空间，用户程序进行处理。数量有限制，遍历复杂。
// nfds: 文件描述符的最大值+1
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

// poll: 与select类似，但是使用链表存储文件描述符，数据结构灵活，数量无限制。pollfd结构体如下：
struct pollfd {
    int fd; // 文件描述符
    short events; // 关注事件类型，POLLIN, POLLOUT, POLLERR, POLLHUP
    short revents; // 实际发生事件类型，POLLIN, POLLOUT, POLLERR, POLLHUP
};
int poll(struct pollfd *fds, nfds_t nfds, int timeout);

// epoll: 不再进行遍历，而是让内核通知示例变化（水平与边缘触发），使用红黑树存储文件描述符，数据结构灵活，数量无限制。
// 创建epoll实例，flag为0表示使用水平触发，EPOLL_CLOEXEC表示在fork后自动关闭，EPOLL_NONBLOCK表示非阻塞。
int epoll_create1(int flag);
// 注册/修改/删除文件描述符
// epfd: epoll实例的文件描述符
// op: EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL 增改删
// fd: 文件描述符
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
struct epoll_event {
    uint32_t events; // 关注事件类型，EPOLLIN, EPOLLOUT, EPOLLERR, EPOLLHUP
    epoll_data_t data; // 用户数据
} __EPOLL_PACHED;
typedef union epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;
// 等待事件发生，获取事件发生的fd
// epfd: epoll实例的文件描述符
// events: 事件数组，存储事件发生的fd
// maxevents: 事件数组的大小
// timeout: 超时时间，-1表示永久阻塞，0表示非阻塞，>0表示超时时间
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

// 读写中
if (ret == -1 && errno == EINTR) { // 客户端正常中断
    printf("fd %d continue reading\n", event[i].data.fd);
    continue;
} else if (ret == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
    printf("fd %d message all read, errno: %d\n", event[i].data.fd, errno);
    break;
}
~~~

## day04 - 包装 - 更加清晰的Socket使用

包装分为Socket, InetAddress, Epoll类，分别实现socket, inet_address, epoll的创建、销毁、注册、等待等函数。

更清晰的epoll服务器Socket工作流程：

1、服务器将Socket A，即监听并等待连接的socket，与ip端口连接，注册到Epoll实例中；

2、当有客户端连接时，Epoll实例将客户端的Socket B（由服务端的socket的accept产生，同时还会获取客户端的ip端口），注册到Epoll实例中；

3、当有客户端发送数据时，Epoll实例分辨出客户端的Socket B，并实现应用；

这个过程中涉及到创建类的实例对象时，可以使用智能指针进行管理。

~~~Cpp
#include <memory>
// 智能指针
std::unique_ptr<Template T> u_ptr(new Object());
~~~