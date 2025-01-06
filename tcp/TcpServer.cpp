/**
 * @file TcpServer.cpp
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief day17-分离了Server创建中手动捆绑Thread与EventLoop的过程。
 * @version 0.1
 * @date 2025-01-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "TcpServer.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "CurrentThread.h"
#include "common.h"
#include <memory>
#include <assert.h>
#include <iostream>


TcpServer::TcpServer(EventLoop *loop, const char * ip, const int port): main_reactor_(loop), next_conn_id_(1){
    // 创建主reactor
    acceptor_ = std::make_unique<Acceptor>(main_reactor_, ip, port);
    std::function<void(int)> cb = std::bind(&TcpServer::HandleNewConnection, this, std::placeholders::_1);
    acceptor_->set_newconnection_callback(cb);

    thread_pool_ = std::make_unique<EventLoopThreadPool>(main_reactor_, 10);

    // std::cout << "Tcpserver listening on " << ip << ":" << port << std::endl;
}

TcpServer::~TcpServer(){
};

void TcpServer::Start(){
    thread_pool_->Start();
    main_reactor_->Loop();
}

inline void TcpServer::HandleNewConnection(int fd){
    assert(fd != -1);
    EventLoop *sub_reactor = thread_pool_->GetNextLoop();
    
    // 创建TcpConnection对象
    std::shared_ptr<TcpConnection> conn = std::make_shared<TcpConnection>(sub_reactor, fd, next_conn_id_);
    std::function<void(const std::shared_ptr<TcpConnection> &)> cb = std::bind(&TcpServer::HandleClose, this, std::placeholders::_1);
    conn->set_connection_callback(on_connect_);

    // 将connection分配给Channel的tie,增加计数
    conn->set_close_callback(cb);
    conn->set_message_callback(on_message_);
    connectionsMap_[fd] = conn;
    // 分配id
    ++next_conn_id_;
    if(next_conn_id_ == 1000){
        next_conn_id_ = 1;
    }
    // 开始监听读事件，on_connect_在这一步调用
    conn->ConnectionEstablished();
}

void TcpServer::SetThreadNum(int num) { thread_pool_->SetThreadNum(num);}

inline void TcpServer::HandleClose(const conn_ptr & conn){
    std::cout <<  CurrentThread::tid() << " TcpServer::HandleClose"  << std::endl;
    main_reactor_->RunOneFunc(std::bind(&TcpServer::HandleCloseInLoop, this, conn));
}

inline void TcpServer::HandleCloseInLoop(const conn_ptr & conn){
    std::cout << CurrentThread::tid()  << " TcpServer::HandleCloseInLoop - Remove connection id: " <<  conn->id() << " and fd: " << conn->fd() << std::endl;
    auto it = connectionsMap_.find(conn->fd());
    assert(it != connectionsMap_.end());
    // 释放Server处的指针计数
    connectionsMap_.erase(connectionsMap_.find(conn->fd()));

    EventLoop *loop = conn->loop();
    // 但是通过绑定指针，计数再加一，直到运行结束，才会释放。
    loop->QueueOneFunc(std::bind(&TcpConnection::ConnectionDestructor, conn));
}

void TcpServer::set_connection_callback(conn_callback const &fn) { on_connect_ = std::move(fn); };
void TcpServer::set_message_callback(conn_callback const &fn) { on_message_ = std::move(fn); };