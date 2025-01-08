/**
 * @file TimerQueue.h
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 类似一个更灵活的Connection类？
 * @version 0.1
 * @date 2025-01-07
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include <functional>
#include "common.h"
#include "TimerStamp.h"
#include <set>
#include <memory>
#include <vector>

class Timer;
class Channel;
class EventLoop;
class TimerQueue
{
public:
    DISALLOW_COPY_AND_MOVE(TimerQueue);
    TimerQueue(EventLoop *loop);
    ~TimerQueue();

    // 插入一个定时器
    void AddTimer(TimerStamp when, std::function<void()> const &cb, double interval = 0.0);
    bool Insert(Timer *timer);

    void CreateTimerfd(); // 创建timerfd

    void ReadTimerFd(); // 处理timerfd读事件
    void HandleRead(); // timerfd可读时，调用

    void ResetTimerFd(Timer *timer); // 重新设置timerfd超时时间，关注新的定时任务
    void ResetTimers(); // 将具有重复属性的定时器重新加入队列

private:
    // 便于按照时间戳排序，越小的越靠前
    typedef std::pair<TimerStamp, Timer *> Entry;
    
    EventLoop *loop_;
    int timerfd_;
    std::unique_ptr<Channel> channel_;

    std::set<Entry> timers_;
    std::vector<Entry> active_timers_; // 运行后的定时器，会保留重复的定时器
};