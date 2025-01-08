#pragma once
#include <time.h>
#include <string>

class TimerStamp
{
public:
    TimerStamp() : micro_seconds_(0) {};
    explicit TimerStamp(int64_t micro_seconds) : micro_seconds_(micro_seconds) {};

    bool operator<(const TimerStamp &rhs) const { return micro_seconds_ < rhs.GetMicroSeconds(); };
    bool operator==(const TimerStamp &rhs) const { return micro_seconds_ == rhs.GetMicroSeconds(); };

    int64_t GetMicroSeconds() const { return micro_seconds_; };

    static TimerStamp Now();
    static TimerStamp AddMicroSec(TimerStamp timestamp, int64_t add_seconds);
    static TimerStamp AddSec(TimerStamp timestamp, double add_seconds);

    std::string GetTimeString() const 
    {
        char buf[64] = {0};
        time_t seconds = static_cast<time_t>(micro_seconds_ / 1000000);
        struct tm tm_time;
        localtime_r(&seconds, &tm_time);
        int microseconds = static_cast<int>(micro_seconds_ % 1000000);
        snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
        return buf;
    };

private:
    int64_t micro_seconds_;
};

inline TimerStamp TimerStamp::Now()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return TimerStamp(ts.tv_sec * 1000000 + static_cast<int64_t>(ts.tv_nsec / 1000));
}

inline TimerStamp TimerStamp::AddMicroSec(TimerStamp timestamp, int64_t add_seconds)
{
    return TimerStamp(timestamp.GetMicroSeconds() + add_seconds);
}

inline TimerStamp TimerStamp::AddSec(TimerStamp timestamp, double add_seconds)
{
    return TimerStamp(timestamp.GetMicroSeconds() + static_cast<int64_t>(add_seconds * 1000000));
}
