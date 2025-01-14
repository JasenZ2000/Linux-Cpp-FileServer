#include "Logger.h"
#include "CurrentThread.h"
#include <utility>

// 为了实现多线程中日志时间格式化的效率，增加了两个__thread变量，
// 用于缓存当前线程存日期时间字符串、上一次日志记录的秒数
__thread char t_time[64];		// 当前线程的时间字符串 “年:月:日 时:分:秒”
__thread time_t t_lastsecond;	// 当前线程上一次日志记录时的秒数

Logger::Impl::Impl(const char *file, int line, LogLevel level)
    : level_(level), line_(line), basename_(Impl::GetBaseName(file))
{
    FormatTime();
    CurrentThread::tid();
    stream_ << Template(CurrentThread::tidString(), CurrentThread::tidStringLength());
    stream_ << Template(logLevelString(), 6);
}

const char *Logger::Impl::GetBaseName(const char *file)
{
    const char *slash = strrchr(file, '/');
    if (slash)
    {
        file = slash + 1;
    }
    return file;
}

void Logger::Impl::FormatTime()
{
    TimerStamp now = TimerStamp::Now();
    time_t seconds = static_cast<time_t>(now.GetMicroSeconds() / 1000000);
    int microseconds = static_cast<int>(now.GetMicroSeconds() % 1000000);

    // 变更日志记录的时间，如果不在同一秒，则更新时间。

    if (t_lastsecond != seconds)
    {
        struct tm tm_time;
        localtime_r(&seconds, &tm_time);
        snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d.",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        t_lastsecond = seconds;
    }

    Fmt us(".%06dZ  ", microseconds);
    stream_ << Template(t_time, 17) << Template(us.data(), 9);
}

void Logger::Impl::Finish()
{
    stream_ << " - " << basename_ << ":" << line_ << "\n";
}

const char* Logger::Impl::logLevelString() const {
    switch(level_) {
        case DEBUG:
            return "DEBUG ";
        case INFO:
            return "INFO  ";
        case WARN:
            return "WARN  ";
        case ERROR:
            return "ERROR ";
        case FATAL:
            return "FATAL ";
        }   
    return nullptr;
} 

Logger::Logger(const char *file, int line, LogLevel level)
    : impl_(file, line, level) {}

void defaultOutput(const char* msg, int len){
    fwrite(msg, 1, len, stdout);  // 默认写出到stdout
}

// Output可能是异步的，而flush则是必须立刻输出
void defaultFlush(){
    fflush(stdout);    // 默认flush到stdout
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;
Logger::LogLevel g_logLevel = Logger::LogLevel::DEBUG;

Logger::~Logger()
{
    impl_.Finish();
    const LogStream::Buffer &buf(stream().buffer());
    g_output(buf.data(), buf.len());
    if (impl_.level_ == FATAL)
    {
        g_flush();
        abort(); 
    }
}

void Logger::setOutput(Logger::OutputFunc func){
    g_output = func;
}

void Logger::setFlush(Logger::FlushFunc func){
    g_flush = func;
}

void Logger::setLogLevel(Logger::LogLevel level){
    g_logLevel = level;
}
