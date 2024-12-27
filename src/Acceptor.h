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
    std::function<void(Socket*)> newConnectionCallback;
public:
    Acceptor(EventLoop* loop);
    ~Acceptor();
    void acceptConnection();
    void setNewConnectionCallback(std::function<void(Socket*)>);
};