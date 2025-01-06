#pragma once
#include "common.h"
#include <functional>
#include <map>
#include <vector>
#include <memory>
class EventLoop;
class TcpConnection;
class Acceptor;
class ThreadPool;

class EventLoopThreadPool;
class TcpServer
{
public:
    typedef std::shared_ptr<TcpConnection> conn_ptr;
    typedef std::function<void(const conn_ptr &)> conn_callback;

    DISALLOW_COPY_AND_MOVE(TcpServer);
    TcpServer(EventLoop *loop, const char *ip, const int port);
    ~TcpServer();

    void Start();

    void set_connection_callback(conn_callback const &fn);
    void set_message_callback(conn_callback const &fn);

    inline void HandleClose(const conn_ptr &);
    // 进行一层额外的封装，以保证erase操作是由`main_reactor_`来操作的。
    inline void HandleCloseInLoop(const conn_ptr &);

    // 接收到消息做的操作。
    inline void HandleNewConnection(int fd);

    void SetThreadNum(int num);

private:
    EventLoop *main_reactor_;
    int next_conn_id_;

    std::unique_ptr<Acceptor> acceptor_;

    // std::unique_ptr<ThreadPool> thread_pool_;
    // std::vector<std::unique_ptr<EventLoop>> sub_reactors_;
    std::unique_ptr<EventLoopThreadPool> thread_pool_;

    std::map<int, conn_ptr> connectionsMap_;

    conn_callback on_connect_;
    conn_callback on_message_;
};
