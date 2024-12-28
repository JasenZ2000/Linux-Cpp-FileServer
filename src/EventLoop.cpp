#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include "ThreadPool.h"
#include <vector>

EventLoop::EventLoop()
    : ep(new Epoll()),
      quit(false),
      tp(new ThreadPool()) {}

EventLoop::~EventLoop() {
    quit = true;
    delete ep;
}

void EventLoop::loop() {
    while (!quit) {
        std::vector<Channel*> activeChannels = ep->poll();
        printf("activeChannels size: %d\n", int(activeChannels.size()));
        for (Channel* channel : activeChannels) {
            channel->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel* channel) {
    ep->updateChannel(channel);
}

void EventLoop::addThread(std::function<void()> func) {
    tp->add(func);
}