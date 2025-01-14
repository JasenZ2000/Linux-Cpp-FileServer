/**
 * @file TcpConnection.cpp
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 原来的写操作会一致阻塞到写完为止，现在只写一次，等待下一次EPOLLOUT事件的触发。
 * @version 0.1
 * @date 2025-01-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "TcpConnection.h"
#include "Buffer.h"
#include "Channel.h"
#include "common.h"
#include "EventLoop.h"
#include "HttpContext.h"
#include "Logger.h"

#include <memory>
#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <sys/socket.h>

TcpConnection::TcpConnection(EventLoop *loop, int connfd, int connid) : connfd_(connfd), connid_(connid), loop_(loop)
{

    if (loop != nullptr)
    {
        channel_ = std::make_unique<Channel>(connfd, loop);
        channel_->EnableET(); // !! 接收信息使用的是边缘触发
        channel_->set_read_callback(std::bind(&TcpConnection::HandleMessage, this));
        channel_->set_write_callback(std::bind(&TcpConnection::HandleWrite, this));
    }
    read_buf_ = std::make_unique<Buffer>();
    send_buf_ = std::make_unique<Buffer>();
    context_ = std::make_unique<HttpContext>();
}

TcpConnection::~TcpConnection()
{
    ::close(connfd_);
}

void TcpConnection::ConnectionEstablished()
{
    state_ = ConnectionState::Connected;
    channel_->Tie(shared_from_this());
    channel_->EnableRead();
    if (on_connect_)
    {
        on_connect_(shared_from_this());
    }
}

void TcpConnection::ConnectionDestructor()
{
    // std::cout << CurrentThread::tid() << " TcpConnection::ConnectionDestructor" << std::endl;
    //  将该操作从析构处，移植该处，增加性能，因为在析构前，当前`TcpConnection`已经相当于关闭了。
    //  已经可以将其从loop处离开。
    loop_->DeleteChannel(channel_.get());
}

void TcpConnection::set_connection_callback(conn_callback const &fn)
{
    on_connect_ = std::move(fn);
}
void TcpConnection::set_close_callback(conn_callback const &fn)
{
    on_close_ = std::move(fn);
}
void TcpConnection::set_message_callback(conn_callback const &fn)
{
    on_message_ = std::move(fn);
}

void TcpConnection::HandleClose()
{
    // std::cout << CurrentThread::tid() << " TcpConnection::HandleClose" << std::endl;
    if (state_ != ConnectionState::Disconected)
    {
        state_ = ConnectionState::Disconected;
        if (on_close_)
        {
            on_close_(shared_from_this());
        }
    }
}

void TcpConnection::HandleMessage()
{
    Read();
    if (on_message_)
    {
        // 要求用户处理业务，正确读取缓存区中的数据。
        on_message_(shared_from_this());
    }
    else
    {
        // 没有用户处理业务，直接将缓存区中的数据清空，避免内存泄漏。
        read_buf_->RetrieveAll();
    }
}

void TcpConnection::HandleWrite()
{
    WriteNonBlocking();
    // 此时channel一定在监听写事件
    if (send_buf_->readablebytes() == 0)
    {
        channel_->DisableWrite();
    }
}

EventLoop *TcpConnection::loop() const { return loop_; }
int TcpConnection::fd() const { return connfd_; }
int TcpConnection::id() const { return connid_; }

TcpConnection::ConnectionState TcpConnection::state() const { return state_; }

Buffer *TcpConnection::read_buf() { return read_buf_.get(); }
Buffer *TcpConnection::send_buf() { return send_buf_.get(); }


void TcpConnection::Send(const char *msg, int len)
{
    send_buf_->Append(msg, len);
    LOG_DEBUG << "TcpConnection::Send - TcpConnection Send " << len << " bytes";
    Write();
    LOG_DEBUG << "TcpConnection::Send - TcpConnection Left " << send_buf_->readablebytes() << " bytes";
    if (send_buf_->readablebytes() > 0)
    {
        channel_->EnableWrite();
    }
}

void TcpConnection::Send(const std::string &msg)
{
    Send(msg.c_str(), static_cast<int>(msg.size()));
}

void TcpConnection::Send(const char *msg)
{
    Send(msg, static_cast<int>(strlen(msg)));
}

void TcpConnection::Read()
{
    ReadNonBlocking();
}

void TcpConnection::Write()
{
    WriteNonBlocking();
}

void TcpConnection::ReadNonBlocking()
{
    char buf[1024];
    while (true)
    {
        memset(buf, 0, sizeof(buf));
        ssize_t bytes_read = read(connfd_, buf, sizeof(buf));
        if (bytes_read > 0)
        {
            read_buf_->Append(buf, bytes_read);
        }
        else if (bytes_read == -1 && errno == EINTR)
        {
            // std::cout << "continue reading" << std::endl;
            continue;
        }
        else if ((bytes_read == -1) && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        {
            break;
        }
        else if (bytes_read == 0)
        { //
            HandleClose();
            break;
        }
        else
        {
            HandleClose();
            break;
        }
    }
}

// 写操作是有可能写着写着，缓冲区满了，就不能再写了，需要等待下一次EPOLLOUT事件的触发。
// 读操作由于数据已经输入内核缓冲区，此时应用程序可以直接从内核缓冲区中读取数据，读到读完为止。
void TcpConnection::WriteNonBlocking()
{
    int size = send_buf_->readablebytes();
    int data_send = static_cast<int>(write(connfd_, send_buf_->Peek(), size));

    // 第一种情况：EWOULDBLOCK = EAGAIN = 11, EAGAIN为Linux统一，写入缓冲区已满，socket非阻塞（当前不能发送）
    if (data_send == -1 && (errno == EWOULDBLOCK || errno == EAGAIN))
        data_send = 0; // 一点都没写进去
    else if (data_send == -1)
        LOG_ERROR << "TcpConnection::Send - TcpConnection Send ERROR";

    LOG_DEBUG << "TcpConnection::Send - TcpConnection Send " << data_send << " bytes, Left " << size - data_send << " bytes";
    send_buf_->Retrieve(data_send);
}
