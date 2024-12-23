#include "Epoll.h"
#include "util.h"
#include <string.h>
#include <unistd.h>
#define max_events 1000

Epoll::Epoll() : epfd(-1), events(nullptr) {
    epfd = epoll_create1(0);
    errif(epfd == -1, "epoll create error");
    events = new epoll_event[max_events];
    bzero(events, sizeof(*events) * max_events);
}

Epoll::~Epoll() {
    delete[] events;
    if(epfd != -1)
        close(epfd);
    epfd = -1;
}

void Epoll::addFd(int fd, uint32_t op) {
    epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.fd = fd;
    ev.events = op;
    errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
}

// 注意默认参数值应在头文件中定义，而不是源文件中，并且不得在源文件中重复定义
std::vector<epoll_event> Epoll::poll(int timeout) {
    std::vector<epoll_event> activeEvents;
    int nums = epoll_wait(epfd, events, max_events, timeout);
    errif(nums == -1, "epoll wait error");
    for (int i = 0; i < nums; ++i) {
        activeEvents.push_back(events[i]);
    }
    return activeEvents;
}