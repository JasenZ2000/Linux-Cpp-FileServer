/**
 * @file Server.h
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 包装了响应已连接客户端的事件
 * @version 0.13
 * @date 2024-12-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#include <map>
#include <vector>
#include <functional>

class EventLoop;
class Socket;
class Acceptor;
class Connection;
class ThreadPool;
class Server{
private:
    EventLoop* mainReactor;
    Acceptor* acceptor;
    std::map<int, Connection*> connections;
    std::vector<EventLoop*> subReactors;
    ThreadPool* thPool;
    std::function<void(Connection*)> connCallback;
public:
    explicit Server(EventLoop*);
    ~Server();
    // set 响应函数时使用std::function，方便使用lambda表达式等函数对象
    void setOnConnectCallback(std::function<void(Connection*)>);
    void newConnection(Socket*);
    void deleteConnection(Socket*);
};