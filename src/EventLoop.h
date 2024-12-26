#pragma once
#include <functional>

class Epoll;
class Channel;
class ThreadPool;
class EventLoop {
private:
    Epoll *ep;
    ThreadPool *tp;
    bool quit;
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void updateChannel(Channel *ch);

    void addThread(std::function<void()>);
};