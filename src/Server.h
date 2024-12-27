#pragma once
#include <map>

class EventLoop;
class Socket;
class Acceptor;
class Connection;
class Server{
private:
    EventLoop* loop;
    Acceptor* acceptor;
    std::map<int, Connection*> connections;
public:
    Server(EventLoop*);
    ~Server();

    void handleReadEvents(int);
    void newConnection(Socket*);
    void deleteConnection(int);
};