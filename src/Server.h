#pragma once
#include <map>
#include <vector>

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
public:
    Server(EventLoop*);
    ~Server();

    void handleReadEvents(int);
    void newConnection(Socket*);
    void deleteConnection(int);
};