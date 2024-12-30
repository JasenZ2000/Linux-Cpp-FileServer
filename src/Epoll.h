#pragma once
#include <sys/epoll.h>
#include <vector>
#include "common.h"

class Channel;
class Epoll {
private:
    int epfd;
    struct epoll_event* events; // 事件数组->epoll_wait
public:
    DISALLOW_COPY_AND_MOVE(Epoll);
    Epoll();
    ~Epoll();

    std::vector<Channel*> poll(long timeout = -1) const;
    void updateChannel(Channel* channel) const;
    void deleteChannel(Channel *channel) const;
};