#pragma once
#include "common.h"
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>

class EventLoop;
class Acceptor;
class Connection;
class ThreadPool;
class Server{
private:
    std::unique_ptr<EventLoop> mainReactor;
    int nextConnid;
    std::unique_ptr<Acceptor> acceptor;

    std::unordered_map<int, Connection*> connections;
    std::vector<std::unique_ptr<EventLoop>> subReactors;
    std::unique_ptr<ThreadPool> thPool;

    std::function<void(Connection*)> connCallback;
    std::function<void(Connection*)> newConnCallback;
public:
    DISALLOW_COPY_AND_MOVE(Server);
    Server(const char *ip, const int port);
    ~Server();

    void start();
    // set 响应函数时使用std::function，方便使用lambda表达式等函数对象
    void setOnConnectCallback(std::function<void(Connection*)> const &fn);
    void setOnNewConnectionCallback(std::function<void(Connection*)> const &fn);

    void newConnection(int);
    void deleteConnection(int);
};