#include "Channel.h"
#include "EventLoop.h"
#include <unistd.h>
#include <sys/epoll.h>

Channel::Channel(EventLoop *_loop, int _fd)
    : loop(_loop),
      fd(_fd),
      events(0),
      revents(0),
      inEpoll(false) {
        // printf("Channel created using fd: %d\n", _fd);
      }

Channel::~Channel()
{
    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }
}

void Channel::handleEvent()
{
    // printf("Channel fd: %d handling event: %d\n", fd, int(revents));
    if (revents & (EPOLLIN | EPOLLRDHUP | EPOLLPRI))
    {
        if (useThreadPool)
            loop->addThread(readCallback);
        else
            readCallback();
    }
    if (revents & EPOLLOUT)
    {
        if (useThreadPool)
            loop->addThread(writeCallback);
        else
            writeCallback();
    }
}

void Channel::enableReading()
{
    events |= EPOLLIN | EPOLLRDHUP | EPOLLPRI;
    loop->updateChannel(this);
}

void Channel::useET()
{
    events |= EPOLLET;
    loop->updateChannel(this);
}

int Channel::getFd()
{
    return fd;
}

uint32_t Channel::getEvents()
{
    return events;
}

uint32_t Channel::getRevents()
{
    return revents;
}

bool Channel::getInEpoll()
{
    return inEpoll;
}

void Channel::setInEpoll(bool _in)
{
    inEpoll = _in;
}

void Channel::setRevents(uint32_t _revents)
{
    revents = _revents;
}

void Channel::setReadCallback(std::function<void()> _cb)
{
    readCallback = _cb;
}

void Channel::setUseThreadPool(bool use)
{
    useThreadPool = use;
}