#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include <vector>
#include <memory>

EventLoop::EventLoop()
    : ep(std::make_unique<Epoll>()),
      quit(false) {}

EventLoop::~EventLoop() {
    quit = true;
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

void EventLoop::removeChannel(Channel* channel) {
    ep->deleteChannel(channel);
}