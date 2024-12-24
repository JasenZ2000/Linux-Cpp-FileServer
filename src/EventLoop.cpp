#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include <vector>

EventLoop::EventLoop()
    : ep(new Epoll()),
      quit(false) {}

EventLoop::~EventLoop() {}

void EventLoop::loop() {
    while (!quit) {
        std::vector<Channel*> activeChannels = ep->poll();
        for (Channel* channel : activeChannels) {
            channel->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel* channel) {
    ep->updateChannel(channel);
}