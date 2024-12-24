#pragma once
#include <functional>

class EventLoop;
class Channel;
class Socket;
class InetAddress;

class Acceptor {
private:
    EventLoop* loop;
    Socket* listenSock;
    Channel* acceptChannel;
    InetAddress* servAddr;
public:
    Acceptor(EventLoop* loop);
    ~Acceptor();
    void acceptConnection();
    std::function<void(Socket*)> newConnectionCallback;
    void setNewConnectionCallback(std::function<void(Socket*)>);
};