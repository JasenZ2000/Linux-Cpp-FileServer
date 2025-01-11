# CppWebServer Learning

跟随github项目，学习C++实现Web服务器。

## 原项目地址
[30daysCppWebServer] https://github.com/Wlgls/30daysCppWebServer

## To Learn Sth More

1、C++异常处理；2、UDP的消息传输；3、非阻塞式socket；4、更好的缓冲区设计；5、线程池优化

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

之前码代码的时候意识到但没有处理的问题零零总总全在这处理了，虽然也没完全处理完，但说实话真挺累人的，一个bug排了半天：

1、Socket与IP中，补上了客户端的connect，IP里也把地址封装到私有了。
2、线程池这一块泛型编程还不能分离实现与声明
3、把所有连进新客户端的连接处理从server拆到Acceptor里了
4、Connection里出了读响应也加了写响应，虽然是空的
5、在开内存的时候使用bzero清空是必要的，它会把内存置为0，避免出现野指针
6、处理请求的时候注意事件类型的处理

## day12 - 主从Reactor模式

这个模式不难理解，主Reactor对应主线程，从Reactor对应子线程，主线程负责监听新连接，子线程负责处理连接的事件。但和前面的代码一比较还是有些个说法的，现在每个线程都分到了一个EventLoop，之前是只有主线程有一个EventLoop，来把任务分配给子线程。执行任务的时候都是一锅粥轮流吃，现在是分了份，每个线程都有自己的碗，各自吃各自的，但主线程还得手动分粥到各个子线程，用了个伪随机（fd取余）处理。这个比喻太抽象了，具体的说每个线程与EventLoop连体，也就是独自和一个Epoll与客户端Connection/Channel连体，拆分程度更高。

比较能预见的问题是某些线程中Connection释放与任务数量的不均衡导致的线程饥饿，现在先不考虑移交任务的问题。

## day13 - 服务器业务逻辑

原来的echo业务是在Connection类中的，而从逻辑上二者是分离的。现在修正Connection的业务逻辑，将业务逻辑从Connection中分离出来，通过回调函数的方式进行处理。在Connection类中较为清晰的将读写两个事件写明了。

又是一场苦战，不难看出来。比想象中轻松，只能说debug一次之后就有经验了。今日的Bug是没把回调函数正确地注册到Channel中，以及读写缓冲区的处理。

## day14 - 代码优化

1、例如EventLoop、Channel等事务与连接相关的类，我们不希望其被复制。要么将其拷贝构造函数与赋值运算符重载函数声明为私有，要么保证其不被自动实现。在其余进行资源创建的时候，尽可能使用移动语义。

~~~cpp
#define DISALLOW_COPY(cname)     \
  cname(const cname &) = delete; \
  cname &operator=(const cname &) = delete;

#define DISALLOW_MOVE(cname) \
  cname(cname &&) = delete;  \
  cname &operator=(cname &&) = delete;

#define DISALLOW_COPY_AND_MOVE(cname) \
  DISALLOW_COPY(cname);               \
  DISALLOW_MOVE(cname);
~~~

2、智能指针。在创建连接、资源时大量使用了无删除的new操作，现在尝试避免内存泄漏。bzero和memset之间还有小小争议。在管理智能指针时，基本上谁创建谁负责，许多的类之间是唯一绑定的，但是也存在野指针管理的情况，尤其是EventLoop需要传递的情况。

~~~cpp
#include <memory>
// 智能指针初始化
std::unique_ptr<Template T> u_ptr(new Object());
// 智能指针获取
u_ptr = std::make_unique<Object>();
// 智能指针获取裸指针，释放
u_ptr.get()
u_ptr.release()
~~~

3、Socket类删掉了，理由是在响应事件的过程中仅需要使用文件描述符，不需要一个Socket类。而在建立连接的过程中虽然需要使用其方法，但全部都在Acceptor类中，无需一个独立的Socket类。

3、卧槽你的，大量的响应函数在Channel、Connection、Server还有Acceptor中间传递，运行的时候还是Epoll查找，Eventloop使Channel开始执行，在一整路向上找到用户的响应函数定义，全麻。

## day15 - Connection的生命周期

之前说的智能指针管理，好处理的对象包括Server以及其对应的主从Reactor，线程，以及acceptor。维度Connection这一个类，由Accpector触发创建事件，由Server的函数进行创建，而删除事件又由Connection本身触发。而由于Connection连接着对应的Channel进行实时事件处理，最糟糕的情况下channel调用响应函数时Connection已经被释放了。导致这一状况的是EventLoop的实时控制要求和Server单一静态管理的矛盾。

解决上述矛盾的方法是用一个shared_ptr，让EventLoop与Server共享Connection，这样就可以在EventLoop中控制Connection的生命周期，而Server则可以通过shared_ptr来管理Connection。

~~~cpp
#include <memory>
// 智能指针初始化
std::shared_ptr<Template T> s_ptr(new Object());
// 智能指针获取
s_ptr = std::make_shared<Object>();
// 对于自定义的类，申明为继承自enable_shared_from_this
class Object : public std::enable_shared_from_this<Object>
// 才可使用shared_from_this()获取shared_ptr
s_ptr_copy = s_ptr.shared_from_this();
// 弱指针，以及将弱指针提升为shared_ptr
std::weak_ptr<Template T> w_ptr;
w_ptr = s_ptr;
std::shared_ptr<Template T> s_ptr_copy = w_ptr.lock();
~~~

muduo的处理：

1.首先连接到来，TcpServer创建TcpConnection，并存入容器。引用计数+1 总数：1

2.客户端断开连接，在Channel的handleEvent函数中会将Channel中的TcpConnection弱指针提升,引用计数+1 总数：2

3.触发HandleRead ，可读字节0，进而触发HandleClose,HandleClose函数中栈上的TcpConnectionPtr guardThis会继续将引用计数+1 总数：3

4.触发HandleClose的回调函数 在TcpServer::removeConnection结束后(回归主线程队列)，释放HandleClose的栈指针，以及Channel里提升的指针引用计数-2 总数：1

5.主线程执行回调removeConnectionInLoop，在函数内部将tcpconnection从TcpServer中保存连接容器中erase掉。但在removeConnectionInLoop结尾用conn为参数构造了bind。引用计数不变 总数：1

6.回归次线程处理connectDestroyed事件，结束完释放参数传递的最后一个shard_ptr，释放TcpConnection。引用计数-1 总数：0

md我EventLoop改到一半改不下去了，输了。ctrlcv一个版本了。由简到繁的梯度曲线在这里有点过于陡了，从一个玩具式的服务器到muduo水平的服务器，基本上主要的改变都在day14-15之间，全麻！

改不下去的一个重要原因是没看懂这个从线程触发关闭连接，而后由主线程上完成Server中连接释放，然后再让子线程(Subreactor)完成连接关闭的过程，后面发现是为了保证connections存储模块的线程安全。另一个重要原因是EventLoop的唤醒事件概念，为了避免来了新任务的时候，EventLoop一直卡在等待唤醒事件，导致任务不执行，带来效率降低。这个对EventLoop的改动相当之大了。

现在这个时候可能是我去接触muduo的时机了。后续的任务基本都脱离服务器本身的框架了，只有日志库可能是会贯穿整个项目，预计还有一个计时器是比较棘手的内容，其余都是比较独立的模块内容。

## day16 - CMakeList

使用更加工程化的cmake生成项目，并通过build目录分离生成对象。

[知乎-CMake使用指南] https://zhuanlan.zhihu.com/p/371257515

## day17 - muduo的多线程实现方案

Muduo的多线程实现方案是基于Reactor模式的多线程实现方案。相对于这里原来的常规线程池，通过绑定reactor(epoll)的loop(循环epoll_wait)来实现多线程的方案，muduo通过EventLoopThreadPool，EventLoopThread和EventLoop的三层封装，实现了更加清晰的多线程实现方案。具体会发生哪些影响，等我先看一看。

1、原来：Server类中创线程池，创EventLoop，在Start时把每个EventLoop的Loop分给子线程去执行。现在：Server类创建线程池以及EventLoopThreadPool，由EventLoopThreadPool创建EventLoopThread，由EventLoopThread创建EventLoop。

2、在具体运行时，EventLoopThreadPool完全运行于主线程上，负责创建EventLoopThread并启动；EventLoopThread在初始化时运行于主线程上，开始时自创子线程，在子线程上创建EventLoop，存在线程安全问题。EventLoop本身则运行于子线程上，负责监听事件并处理。

3、原版muduo还是不小的，自己实现了Thread，通过隔离Thread的注册与开始，让逻辑上更加清晰。这里就不能直接参考，C++默认THread是注册即开始的。

## day18 - HTTP协议

两种HTTP解析方案：原项目的状态转换机，以及目前正在参考的另一个cpp-httplib库（逐行解析）。好像一般是状态转换机相对性能更高，node.js也是用的这种（llhttp）。Http请求报文包括请求行、请求头、请求体，请求体之前还有空行。

~~~
GET /HEELO HTTP/1.1\r\n
Host: 127.0.0.1:1234\r\n
Connection: Keep-alive\r\n
Content-Length: 12\r\n
\r\n
hello world;
~~~

HTTP请求的解析结果包括以下内容：

~~~cpp
class HttpRequest{
private:
    Method method_; // 请求方法
    Version version_; // HTTP版本
    std::map<std::string, std::string> request_params_; // 请求参数
    std::string url_; // 请求路径
    std::string protocol_; // 请求协议
    std::map<std::string, std::string> headers_; // 请求头
    std::string body_; // 请求体
}
~~~

非常纯粹的重复工作，逐字符读入判断状态变化，还挺逆天的。最大的问题是对每个字符都要进行状态判断，效率低下。果然还是分解成内部循环更加符合我刷算法题的直觉，就不在这里写了，要吐出来了。

## day19 - HTTP服务器

HTTP的服务端响应报文包括响应行、响应头、响应体，同样在响应体之前有空行。

~~~
HTTP/1.1 200 OK\r\n
Content-Encoding: gzip\r\n
Content-Type: text/html\r\n
Content-Length: 5\r\n
\r\n
hello
~~~

其实读入的Http报文也算是一种缓存吧，写到了COnnection里面，挺神奇的。其他真的没啥，主要就是在TCP上加入了固定的Http输入输出格式，比较简单的封装。

## day20 - 定时器

按时间触发事件，而不只是依赖于事件的触发，即是一个定时任务。不过在代码上似乎靠的是linux的timerfd，结果上类似于时间的触发？Muduo的定时器通过Timer、TimerQueue以及TimerId来实现，而TimerID是用户接口，Timer类保存超时时刻，回调函数以及类型，TimerQueue则是管理Timer的类，基于set，也就是红黑树管理。

~~~cpp
#include <sys/timefd.h>
timerfd_ = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
timerfd_settime(tfd, 0, &new_value, NULL)
~~~

TimerQueue通过channel绑定timerfd，再交由epoll监听并处理，但在one loop per thread的模式下该如何处理呢？答案是给每个Loop配一个TimerQueue。

~~~cpp
#include <time.h>
// 纳秒级计时器 ts.tv_sec ts.tv_nsec，还可设定时钟类型
struct timespec ts;
clock_gettime(CLOCK_MONOTONIC, &ts);
// 微秒级计时器 time.tv_sec time.tv_usec
struct timeval time;
gettimeofday(&time, NULL);
~~~

顺手把服务器主动关闭连接也写了吧。这个过程还是有点复杂的。

1、Acceptor在主线程中创建Connection并把创建响应函数绑给Connection，Connection运行在从线程上，并在从线程上调用创建响应函数。我们需要让从线程的EventLoop运行该Connection对应的定时任务。

2、接下来好像就没啥问题了，和之前的Connection关闭连接是相同的运行逻辑，毕竟Loop和线程都相同，直接调函数就行。啥没问题啊，定时器超时相应的时候Connection释放了咋办呢？还是得针对于shared_ptr得到一个weak_ptr在手上进行判断。

## day22 - 日志库

### 输出流

~~~cpp
class LogStream
{
    // 对应实现 log << 2 log << 'c' 这种情况
    LogStream &operator<<(int);
    LogStream &operator<<(char);
}
// 对应实现 log << Fmt("hello %s", "world") 这种格式化输出情况
inline LogStream & operator<<(LogStream& s, const Fmt& fmt){
    s.append(fmt.data(), fmt.length());
    return s;
};
~~~

### 日志库接口

日志等级：DEBUG、INFO、WARN、ERROR、FATAL，本质上通过宏来快速获得对应的日志输出流，取代std::cout，实现一个便捷的接口。

~~~cpp
#define LOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) \
    Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
    Logger(__FILE__, __LINE__, Logger::INFO).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()
~~~

### 异步日志

我们知道写入文件的速度是非常慢的，这一过程中会发生较长时间的阻塞。所以一方面我们需要将写入日志独立为一个线程，一方面在主线程中使用缓冲区来缓存日志，当缓冲区满了之后再交给从线程处理。标准生产者消费者模型，主线程生产，从线程消费。牺牲了一定的实时性，增加了丢日志风险。

双缓冲：一个缓冲区满了之后，将其与另一个缓冲区交换，然后再将另一个缓冲区交给从线程处理。这样可以避免主线程在处理日志的时候，从线程也在处理日志，造成冲突。在这里最好让从线程处理日志的频率大些，避免出现主线程写满后，从线程还没处理完的情况，导致阻塞。

日志的具体流向：

1、异步接口与同步接口一致，都是在主线程中通过宏获取日志输出流Logger::Impl::LogStream，将单条日志写入LogStream的小型缓冲区。

2、LogStream缓冲区内的小型日志会在Logger::~Logger()调用g_output，将data,length写入到目标位置，FATAL级别调用g_flush输出 

3、对于同步来说，直接写到stdout就好，主线程自己生产自己输出；但对于异步来说，则是将日志写入大型缓冲区，通过判断大型缓冲区是否满了，满了则调用调用LogFile输出到文件中。

4、具体而言，主线程写入大型缓冲区，判断是否满了，满了则丢到从线程的处理队列，给条件变量发信号，唤醒从线程。从线程处理队列，将日志写入文件。这种异步日志假设只有主线程在写入日志，在多线程的情况下，有线程安全问题。