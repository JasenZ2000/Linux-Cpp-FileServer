#include "TimerQueue.h"
#include "Timer.h"
#include "Channel.h"
#include "EventLoop.h"
#include <sys/timerfd.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

TimerQueue::TimerQueue(EventLoop *loop) 
    : loop_(loop), timers_(std::set<Entry>())
{
    CreateTimerfd();
    channel_ = std::make_unique<Channel>(timerfd_, loop_);
    channel_->set_read_callback(std::bind(&TimerQueue::HandleRead, this));
    channel_->EnableRead();
}

void TimerQueue::CreateTimerfd()
{
    timerfd_ = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    assert(timerfd_ >= 0);
}

TimerQueue::~TimerQueue()
{
    loop_->DeleteChannel(channel_.get());
    close(timerfd_);
    for (const auto &Entry : timers_)
    {
        delete Entry.second;
    }
}

void TimerQueue::HandleRead(){
    ReadTimerFd();
    active_timers_.clear();
    
    // 找到所有到期的定时器
    auto end = timers_.lower_bound(Entry(TimerStamp::Now(), reinterpret_cast<Timer *>(UINTPTR_MAX)));
    active_timers_.insert(active_timers_.end(), timers_.begin(), end);

    timers_.erase(timers_.begin(), end);
    for (const auto &entry : active_timers_)
    {
        entry.second->Run();
    }
    ResetTimers();
}

void TimerQueue::ResetTimerFd(Timer *timer)
{
    struct itimerspec new_value;
    struct itimerspec old_value;
    bzero(&new_value, sizeof(new_value));
    bzero(&old_value, sizeof(old_value));

    int64_t micro_seconds = timer->GetExpireTime().GetMicroSeconds() - TimerStamp::Now().GetMicroSeconds();
    if (micro_seconds < 100)
    {
        micro_seconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(micro_seconds / 1000000);
    ts.tv_nsec = static_cast<long>(micro_seconds % 1000000 * 1000);
    new_value.it_value = ts;

    int ret = ::timerfd_settime(timerfd_, 0, &new_value, &old_value);

    if (ret < 0)
    {
        printf("timerfd_settime error!\n");
    }
}

void TimerQueue::ResetTimers()
{
    for (const auto &Entry : active_timers_)
    {
        if (Entry.second->Repeat())
        {
            Entry.second->Restart(TimerStamp::Now());
            Insert(Entry.second);
        }
        else
        {
            delete Entry.second;
        }
    }
    if (!timers_.empty())
    {
        ResetTimerFd(timers_.begin()->second);
    }
}

void TimerQueue::AddTimer(TimerStamp when, std::function<void()> const &cb, double interval)
{
    Timer *timer = new Timer(when, cb, interval);
    if (Insert(timer))
    {
        ResetTimerFd(timer);
    }
}

bool TimerQueue::Insert(Timer *timer)
{
    bool reset_instantly = false;
    if(timers_.empty()){
        reset_instantly = true;
    }
    else if(timer->GetExpireTime() < timers_.begin()->first)
    {
        reset_instantly = true;
    }
    timers_.emplace(std::move(Entry(timer->GetExpireTime(), timer)));
    return reset_instantly;
}

void TimerQueue::ReadTimerFd(){
    uint64_t read_byte;
    ssize_t readn = ::read(timerfd_, &read_byte, sizeof(read_byte));
    if(readn!= sizeof(read_byte)){
        printf("TimerQueue::ReadTimerFd read error");
    }
}
