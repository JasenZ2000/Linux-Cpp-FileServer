#pragma once
#include <functional>

class EventLoop;
class Socket;
class Channel;
class Connection {
private:
    EventLoop* loop;
    Socket* sock;
    Channel* channel;
    std::function<void(Socket*)> deleteConnectionCallback;
public:
    Connection(EventLoop*, Socket*);
    ~Connection();

    void echo(int sockfd);
    void setDeleteConnectionCallback(std::function<void(Socket*)>);
};