# CppWebServer Learning

跟随github项目，学习C++实现Web服务器。

## 原项目地址
[30daysCppWebServer] https://github.com/Wlgls/30daysCppWebServer

## To Learn Sth More

1、C++异常处理；2、UDP的消息传输；3、非阻塞式socket；4、更好的缓冲区设计

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

IO复用是指通过单个线程，同时监听多个IO事件，当有IO事件发生时，通知程序进行处理。在Linux中，IO复用的实现有select、poll、epoll等。通过epoll的IO复用，也就是通过单一的监控线程，去把握当前有哪些连接是需要处理的。没有epoll时，需要为每个连接创建一个线程，每个线程可能还需要通过堵塞/轮询的方式监控变化，这样会造成线程数量过多，导致线程切换的开销过大。

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

这个过程中涉及到创建类的实例对象时，可以使用智能指针进行管理。然而生命周期的管理比你想的复杂。

~~~Cpp
#include <memory>
// 智能指针
std::unique_ptr<Template T> u_ptr(new Object());
~~~

## day05 - epoll与Channel

这里的Channel实际是对epoll的监听的封装。将原本的从epoll描述符去接触事件，通过Channel从事件的角度进行封装，涵盖了开始监听、调整监听事件、关闭等功能，今日不算关键。这种Channel厉害之处在于可以把处理函数指针也包装上，使得Channel可以处理多种事件，还能与线程池结合，实现多线程处理。

## day06 - 服务器Reactor开发模式

Reactor模式是一种事件驱动的模式，和上面的Channel有着一致的思想，分为单Reactor(单进程/多线程)和多Reactor。单Reactor模式是指只有一个Reactor线程，所有的IO事件都由这个线程处理。多Reactor模式是指有多个Reactor线程，每个Reactor线程处理一个IO事件。

单Reactor单进程/线程：

Reactor监听事件，通过dispatch分发事件到Acceptor（建立连接）或Handler（响应任务），调用相应的处理函数。

单Reactor多线程：

心心念念的线程池技术终于出现了，Handler对象在这里不执行任务，而是将任务与数据放入线程池，由线程池中的Processor进行处理，再返回到Handler，再返回给客户。

多Reactor： 开源项目Netty，Memcache中使用的是多Reactor多线程模式。

多Reactor模式是指有多个Reactor线程，每个Reactor线程处理一个IO事件。在主线程里有一个MainReactor，而在每个子线程里有一个SubReactor。

Day06的代码基本上只是进一步的封装，还在打牢基础。今日实现Server类，先在类中完成Accpector与Handler的任务，核心还是其本身的监听任务，通过EventLoop来实现。(EPOLLRDHUP是连接切断时的事件，想第一时间切掉的话就得监听)

写完之后发现比想象中的改的多：Channel如我所想包圆了事件的初始化与处理的实现，EventLoop是通过epoll实时访问活跃Channel的运行包装，Server类则是初始化的大集合。

## day07 - Acceptor

其实就是把新建连接这一块的内容拆出来独立封装，从功能角度而言是独立且关键的部分，但目前表现不出影响。要说的话，基于functional与bind的函数对象传递是非常实用的写法。

~~~cpp
#include <functional>
// C++11 提供的通用函数封装工具，适用普通函数、函数对象、lambda表达式以及成员函数。
// void ( int ) 返回值为void，参数为int的函数对象
std::function<void(int)> func;
// 用bind绑定成员函数以及参数，分别需要目标函数地址，类对象地址，参数
func = std::bind(&MyClass::MyFunc, &myObj, std::placeholders::_1);
~~~

## day08 - Connection

解决房子里的大象：直到目前服务器都没有存储与客户端的连接信息，仅仅是在每个客户端连进来后，拿Channel把客户端的目标事件包装进了EventLoop里运行。预判错误，实际上是和Acceptor类似的，对执行任务与响应方面的封装。

将每个连接的信息、状态、事件都封装进了Connection里，既实现了与客户端的连接信息的存储，又实现了对各个客户端连接的逻辑的封装。

MD源码有问题，新连接都没有创建对应的Socket和InetAddress，把监听新连接的Socket鱼目混珠到里面了，烂完了。

这个版本标志着服务器的核心功能已经完成，2024.12.20 -> 2024.12.25: 第一阶段。

## day09 - 缓冲区

使用Buffer前，每次读取数据都以相同的大小进行，非常原始，存在大量冗余的内存操作。给每个Connection配了一个最最简单的缓冲区，实现的功能更接近与动态长度输入下的消息汇总，还没涉及到优化内存管理。

## day10 - 线程池

这里采用固定线程数的线程池，其应当注意两点：1、对任务队列读写的互斥；2、任务队列为空时避免轮询。两个问题分别对应到mutex和condition_variable。

~~~cpp
#include <mutex>
std::mutex mtx;
// 加锁：lock, unlock, try_lock都是基本函数
// lock_guard是RAII风格的锁，构造时加锁，析构时解锁，作用域结束后自动解锁。
std::lock_guard<std::mutex> lock(mtx);
// unique_lock是更灵活的锁，构造时加锁，析构时解锁，作用域结束后自动解锁。支持手动解/加锁，尝试/延迟加锁，条件变量协作等。
std::unique_lock<std::mutex> lock(mtx);

#include <condition_variable>
std::condition_variable cv;
// 等待：wait, wait_for, wait_until 等待到条件成立，wait_for和wait_until可以指定等待时间。
cv.wait(lock, []{ return !task_queue.empty(); });
cv.notify_one(); // 唤醒一个等待的线程，与之前的条件判断配合，保证虚假唤醒不影响程序的正常运行。
~~~

这个版本把所有任务打包成std::function<void()>的方式包装可执行任务，并通过线程池进行调度。缺陷则是没有返回值。

加入了线程池之后回顾我们这一个项目，发现以EventLoop为核心的事件触发与处理，以及基于Acceptor，Connection的Server功能实现，以及有了较为明显的区分。这两个部分之间的交流是基于Channel类来实现的，Channel里面包含了Socket，事件以及处理函数，是服务器工作的最小单元。

day11 - 线程池的优化

线程池的优化是一个非常庞大的话题，这里处理两个方面的优化：任务队列避免复制，响应函数的返回值处理。

这里的处理用到了泛型编程，将返回值与函数参数类别作为模板输入。

~~~cpp
template <class F, class... Args>
// F:可调用（函数）类型，Args:可调用的参数类型 返回一个future，其利用result_of获取可调用对象的返回类型。
auto ThreadPool::add(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;
    // 外包智能指针负责传递与调用中的内存管理，内包packaged_task负责后续用get_future()绑定到future
    // 使用bind绑定函数f与参数，生成一个无参的可调用对象，返回一个future
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(mtx);

        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");
        tasks.emplace([task]() { (*task)(); });
    }
    cv.notify_one();
    return res;
}
~~~

之前码代码的时候意识到但没有处理的问题零零总总全在这处理了，虽然也没完全处理完，但说实话真挺累人的：

1、Socket与IP中，补上了客户端的connect，IP里也把地址封装到私有了。
2、线程池这一块泛型编程还不能分离实现与声明
3、把所有连进新客户端的连接处理从server拆到Acceptor里了
4、Connection里出了读响应也加了写响应，虽然是空的