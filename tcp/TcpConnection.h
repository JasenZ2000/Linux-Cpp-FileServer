#pragma once
#include "common.h"
#include "TimerStamp.h"
#include <functional>
#include <memory>
#include <string>
class Buffer;
class HttpContext;
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    typedef std::function<void(const std::shared_ptr<TcpConnection> &)> conn_callback;

    enum ConnectionState
    {
        Invalid = 1,
        Connected,
        Disconected
    };

    DISALLOW_COPY_AND_MOVE(TcpConnection);

    
    TcpConnection(EventLoop *loop, int connfd, int connid);
    ~TcpConnection();

    // 初始化TcpConnection
    void ConnectionEstablished();

    // 销毁TcpConection
    void ConnectionDestructor();

    // 建立连接时调用回调函数
    void set_connection_callback(conn_callback const &fn);
     // 关闭时的回调函数
    void set_close_callback(conn_callback const &fn);   
    // 接受到信息的回调函数                                  
    void set_message_callback(conn_callback const &fn); 

    // 设定send buf
    void set_send_buf(const char *str); 

    Buffer *read_buf();
    Buffer *send_buf();

    /// @brief 读到socket缓冲区为空
    void Read();
    void Write(); // 写操作

    void Send(const std::string &msg); // 输出信息
    void Send(const char *msg, int len); // 输出信息
    void Send(const char *msg);

    void HandleMessage(); // 当接收到信息时，进行回调

    // 当TcpConnection发起关闭请求时，进行回调，释放相应的socket.
    void HandleClose(); 

    ConnectionState state() const;
    EventLoop *loop() const;
    int fd() const;
    int id() const;

    HttpContext *context() const { return context_.get(); };

    TimerStamp GetActTime() { return timerstamp_; };
    void SetActTime(TimerStamp ts) { timerstamp_ = ts; };

private:
    // 该连接绑定的Socket
    int connfd_;
    // 为该连接分配一个id方便后续的debug
    int connid_;

    // 连接状态
    ConnectionState state_;

    EventLoop *loop_;

    std::unique_ptr<Channel> channel_;
    std::unique_ptr<Buffer> read_buf_;
    std::unique_ptr<Buffer> send_buf_;

    std::unique_ptr<HttpContext> context_;

    conn_callback on_close_;
    conn_callback on_message_;
    conn_callback on_connect_;

    void ReadNonBlocking();
    void WriteNonBlocking();

    TimerStamp timerstamp_;
};