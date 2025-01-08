/**
 * @file Timer.h
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 包装定时器类，其实就是保存了一个时间戳、回调函数和状态
 * @version 0.1
 * @date 2025-01-07
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once
#include "common.h"
#include <functional>
#include "TimerStamp.h"

class Timer
{
public:
    using TimerCallback = std::function<void()>;
    DISALLOW_COPY_AND_MOVE(Timer);
    
    Timer(TimerStamp ts, TimerCallback const &cb, double interval = 0.0)
        : expireTime_(ts), callback_(cb), interval_(interval), repeat_(interval > 0.0) {};
        
    TimerStamp GetExpireTime() const { return expireTime_; }
    bool Repeat() const { return repeat_; }

    void Run() const { callback_(); };

    void Restart(TimerStamp ts) { if (repeat_) expireTime_ = TimerStamp::AddSec(ts, interval_);};

private:
    TimerStamp expireTime_;
    TimerCallback callback_;
    double interval_;
    bool repeat_;
    
};