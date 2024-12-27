#include "Epoll.h"
#include "Channel.h"
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
std::vector<Channel*> Epoll::poll(int timeout) {
    std::vector<Channel*> activeEvents;
    int nums = epoll_wait(epfd, events, max_events, timeout);
    errif(nums == -1, "epoll wait error");
    for (int i = 0; i < nums; ++i) {
        Channel* ch = (Channel*)events[i].data.ptr;
        printf("channel fd: %d now get event: %d\n", ch->getFd(), int(events[i].events));
        ch->setRevents(events[i].events);
        activeEvents.push_back(ch);
    }
    return activeEvents;
}

void Epoll::updateChannel(Channel* channel) {
    int fd = channel->getFd();
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel; // 这句与上面的 poll 函数的 events 数组对应，传回信息更多的Channel，不仅仅fd
    ev.events = channel->getEvents();
    if (channel->getInEpoll()) {
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll update error");
    } else {
        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
        channel->setInEpoll();
    }
}

void Epoll::deleteChannel(Channel *channel){
    int fd = channel->getFd();
    errif(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1, "epoll delete error");
    channel->setInEpoll(false);
}